#pragma once
#include "kart_remote.h"
#include "kart_nitro.h"
class KartDrive {
public:
    KartRemote remote;
    KartNitro nitro;
    float x=36,z=1.1f,yaw=0,speed=0,steer=0;
    int lap=0,checkpoint=0; bool exit=false;
    void update(){
        auto now=std::chrono::steady_clock::now();
        float dt=std::min(.04f,std::chrono::duration<float>(now-previous).count());previous=now;
        nitro.tick(dt);
        auto input=remote.read(speed+nitro.extra(),lap);
        if(input.command==2){exit=true;speed=0;nitro.cancel();return;}
        if(input.command==1){x=36;z=1.1f;yaw=speed=steer=0;lap=checkpoint=0;lastAngle=0;nitro.reset();return;}
        if(!input.connected){speed=0;steer=0;nitro.cancel();return;}
        // Stronger small-stick response, with smoothing to avoid sudden direction changes.
        float target=-std::copysign(std::pow(std::abs(input.steer),.7f),input.steer);
        steer+=(target-steer)*(1-std::exp(-13*dt));
        float norm=std::hypot(x/36.f,z/24.f);bool road=std::abs(norm-1)<.145f;
        if(input.brake>.1f)nitro.cancel();
        if(!nitro.active()){
        if(input.brake>.1f){
            if(speed>0) speed=std::max(0.f,speed-18*input.brake*dt);
            else speed-=4*input.brake*dt;
        }
        else if(input.gas>.1f){
            if(speed<0) speed=std::min(0.f,speed+18*input.gas*dt);
            else if(speed<(road?13.f:10.f)) speed=std::min(road?13.f:10.f,speed+7*input.gas*dt);
        }
        else speed*=std::exp(-(road?.35f:.6f)*dt);
        // Ease into the grass speed limit instead of abruptly clamping on crossing the curb.
        if(!road && speed>10.f) speed=10.f+(speed-10.f)*std::exp(-1.5f*dt);
        speed=std::max(-3.f,std::min(13.f,speed));
        }
        float actual=speed+nitro.extra(),oldX=x,oldZ=z;
        yaw+=steer*actual*.13f*dt;x+=std::sin(yaw)*actual*dt;z+=std::cos(yaw)*actual*dt;
        if(speed>0)nitro.collect(oldX,oldZ,x,z);
        float boundary=std::hypot(x/42.f,z/30.f);if(boundary>1){x/=boundary;z/=boundary;speed=0;nitro.cancel();}
        float angle=std::atan2(z/24.f,x/36.f);if(angle<0)angle+=6.283185f;
        if(road&&speed>0){if(checkpoint==0&&angle>1.57f&&angle<2.51f)checkpoint=1;
            else if(checkpoint==1&&angle>3.14f&&angle<4.08f)checkpoint=2;
            else if(checkpoint==2&&angle>4.71f&&angle<5.65f)checkpoint=3;
            else if(checkpoint==3&&lastAngle>5.5f&&angle<.5f){lap++;checkpoint=0;}}
        lastAngle=angle;
    }
private:
    float lastAngle=0;
    std::chrono::steady_clock::time_point previous=std::chrono::steady_clock::now();
};


