#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <cmath>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <poll.h>
#include <fstream>
#include <sys/stat.h>
#include "controller_page.h"

class KartRemote {
public:
    struct Input {float steer=0,gas=0,brake=1; int command=0; bool connected=false;};
    KartRemote(){std::random_device r; token=std::to_string(r())+std::to_string(r()); std::ifstream saved("/data/user/0/com.deeptime.xrkart.opengles/files/pairing.txt");
        saved>>deviceId>>pairCode;
        if(deviceId.empty()||pairCode.size()!=6){deviceId=std::to_string(r())+std::to_string(r());pairCode=std::to_string(100000+r()%900000);mkdir("/data/user/0/com.deeptime.xrkart.opengles/files",0700);std::ofstream out("/data/user/0/com.deeptime.xrkart.opengles/files/pairing.txt");out<<deviceId<<" "<<pairCode;out.close();chmod("/data/user/0/com.deeptime.xrkart.opengles/files/pairing.txt",0600);}
        worker=std::thread([this]{run();}); discovery=std::thread([this]{discover();});}
    ~KartRemote(){stop=true;if(worker.joinable())worker.join();if(discovery.joinable())discovery.join();}
    Input read(float speed,int lap){std::lock_guard<std::mutex> guard(lock); currentSpeed=speed;currentLap=lap;Input result=input;input.command=0;result.connected=true;if(std::chrono::steady_clock::now()-last>std::chrono::milliseconds(900)){result.steer=0;result.gas=0;result.brake=1;result.connected=false;}return result;}
    std::string deviceId,pairCode;
    std::atomic<bool> exitRequested{false};
private:
    std::atomic<bool> stop{false};std::thread worker,discovery;std::mutex lock;Input input;
    float currentSpeed=0;int currentLap=0;std::string token,owner; unsigned long lastSeq=0;
    std::chrono::steady_clock::time_point last{};
    void discover(){
        int fd=socket(AF_INET,SOCK_DGRAM,0);if(fd<0)return;
        sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(8089);addr.sin_addr.s_addr=INADDR_ANY;
        if(bind(fd,(sockaddr*)&addr,sizeof(addr))<0){close(fd);return;}
        while(!stop){pollfd p{fd,POLLIN,0};if(poll(&p,1,200)<=0)continue;char buf[128];sockaddr_in from{};socklen_t size=sizeof(from);int n=recvfrom(fd,buf,sizeof(buf),0,(sockaddr*)&from,&size);
            if(n==16&&std::string(buf,n)=="APEX_DISCOVER_V1"){std::string reply="APEX_XR_V1 "+deviceId+" 8088";sendto(fd,reply.data(),reply.size(),0,(sockaddr*)&from,size);}
        }close(fd);
    }
    void run(){
        int server=socket(AF_INET,SOCK_STREAM,0);if(server<0)return;
        int reuse=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
        sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_port=htons(8088);addr.sin_addr.s_addr=INADDR_ANY;
        if(bind(server,(sockaddr*)&addr,sizeof(addr))<0||listen(server,4)<0){close(server);return;}
        while(!stop){pollfd p{server,POLLIN,0};if(poll(&p,1,100)<=0)continue;int fd=accept(server,nullptr,nullptr);if(fd<0)continue;
            int noDelay=1;setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&noDelay,sizeof(noDelay));
            timeval timeout{0,200000};setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
            std::string req;char buf[2048];size_t split=std::string::npos;int length=0;
            while(req.size()<8192){int n=recv(fd,buf,sizeof(buf),0);if(n<=0)break;req.append(buf,n);split=req.find("\r\n\r\n");if(split!=std::string::npos){std::string lower=req;std::transform(lower.begin(),lower.end(),lower.begin(),[](unsigned char c){return std::tolower(c);});auto pos=lower.find("content-length:");if(pos!=std::string::npos)length=atoi(req.c_str()+pos+15);if(length<0||length>1024)break;if(req.size()>=split+4+length)break;}}
            std::string body="Not found",type="text/plain",status="404 Not Found";
            if(req.rfind("GET /identity HTTP/",0)==0){body="{\"id\":\""+deviceId+"\",\"name\":\"APEX XR\"}";type="application/json";status="200 OK";}
            else if(req.rfind("GET / HTTP/",0)==0){
                std::string headers=req.substr(0,split);std::transform(headers.begin(),headers.end(),headers.begin(),[](unsigned char c){return std::tolower(c);});
                headers+="\r\n";
                if(headers.find("\r\nx-apex-pin: "+pairCode+"\r\n")==std::string::npos){body="Pairing required";status="403 Forbidden";}
                else{body=kControllerPage;auto pos=body.find("__TOKEN__");body.replace(pos,9,token);type="text/html; charset=utf-8";status="200 OK";}}
            else if(req.rfind("POST /input HTTP/",0)==0&&split!=std::string::npos){std::istringstream stream(req.substr(split+4));std::string key,client;unsigned long seq;Input next;
                if(stream>>key>>client>>seq>>next.steer>>next.gas>>next.brake>>next.command && key==token&&std::isfinite(next.steer)&&std::isfinite(next.gas)&&std::isfinite(next.brake)&&next.command>=0&&next.command<=2){
                    std::lock_guard<std::mutex> guard(lock);
                    bool available=owner.empty()||owner==client||std::chrono::steady_clock::now()-last>std::chrono::seconds(2);
                    if(!available){status="409 Conflict";body="Controller in use";}
                    else if(owner==client&&seq<=lastSeq){status="409 Conflict";body="Stale input";}
                    else{owner=client;lastSeq=seq;next.connected=true;next.steer=std::max(-1.f,std::min(1.f,next.steer));next.gas=std::max(0.f,std::min(1.f,next.gas));next.brake=std::max(0.f,std::min(1.f,next.brake));if(next.command==0)next.command=input.command;input=next;last=std::chrono::steady_clock::now();
                    body="{\"speed\":"+std::to_string(currentSpeed)+",\"lap\":"+std::to_string(currentLap)+"}";type="application/json";status="200 OK";}
                }else{status="403 Forbidden";body="Invalid input";}}
            std::string response="HTTP/1.1 "+status+"\r\nContent-Type: "+type+"\r\nContent-Length: "+std::to_string(body.size())+"\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n"+body;
            size_t offset=0;while(offset<response.size()){int n=send(fd,response.data()+offset,response.size()-offset,MSG_NOSIGNAL);if(n<=0)break;offset+=n;}close(fd);
        }close(server);
    }
};

