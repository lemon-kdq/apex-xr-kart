#pragma once
#include <array>
#include <random>
#include <cmath>
#include <algorithm>

class KartNitro {
public:
    struct Bottle {float x,z,cooldown=0;};
    std::array<Bottle,8> bottles{};
    float age=99,clock=0,hitX=0,hitZ=0,effect=99,boostFrom=0,holdExtension=0;
    KartNitro(){reset();}
    void reset(){age=effect=99;boostFrom=holdExtension=0;for(int i=0;i<8;i++)spawn(i);}
    bool active() const{return age<3.2f+holdExtension;}
    static float smooth(float t){t=std::max(0.f,std::min(1.f,t));return t*t*(3-2*t);}
    float extra() const{if(age<.4f)return boostFrom+(12-boostFrom)*smooth(age/.4f);if(age<1.9f+holdExtension)return 12;if(age<3.2f+holdExtension)return 12*(1-smooth((age-1.9f-holdExtension)/1.3f));return 0;}
    void cancel(){age=99;boostFrom=holdExtension=0;}
    void tick(float dt){clock+=dt;age+=dt;effect+=dt;for(int i=0;i<8;i++)if(bottles[i].cooldown>0){bottles[i].cooldown-=dt;if(bottles[i].cooldown<=0)spawn(i);}}
    bool collect(float ax,float az,float bx,float bz){

        for(auto& b:bottles){if(b.cooldown>0)continue;float dx=bx-ax,dz=bz-az,len=dx*dx+dz*dz;
            float t=len>.00001f?std::max(0.f,std::min(1.f,((b.x-ax)*dx+(b.z-az)*dz)/len)):0;
            if(std::hypot(b.x-ax-t*dx,b.z-az-t*dz)<1.8f){if(active()&&age<1.9f+holdExtension){holdExtension+=1.5f;}else{boostFrom=extra();holdExtension=0;age=0;}effect=0;hitX=b.x;hitZ=b.z;b.cooldown=8;return true;}}
        return false;
    }
    void draw(std::vector<Cube>& out,float carX,float carZ,float yaw){
        auto box=[&](float x,float y,float z,float sx,float sy,float sz,float a,XrVector3f color){out.push_back(Cube{{{0,std::sin(a/2),0,std::cos(a/2)},{x,y,z}},{sx,sy,sz},color});};
        for(int i=0;i<8;i++){auto& b=bottles[i];if(b.cooldown>0)continue;float y=1.25f+.18f*std::sin(clock*2+i),a=clock*.8f+i;
            for(int side=0;side<4;side++)box(b.x,y,b.z,.70f,1.5f,.70f,a+side*.785398f,{.04f,.75f,1});
            box(b.x,y+.92f,b.z,.42f,.35f,.42f,a,{.8f,.95f,1});
            box(b.x,y,b.z,.9f,.25f,.9f,a,{.85f,1,1});
            for(int j=0;j<12;j++){float t=j*.523599f;box(b.x+std::sin(t)*1.1f,.11f,b.z+std::cos(t)*1.1f,.3f,.04f,.12f,t,{.05f,.8f,.95f});}}
        if(effect<.85f){float k=effect/.85f;for(int i=0;i<24;i++){float a=i*2.39996f,r=(1+effect*6)*(i%3+1)/3.f,s=.28f*(1-k);box(hitX+std::sin(a)*r,.5f+std::sin(k*3.14159f)*(1+i%4*.3f),hitZ+std::cos(a)*r,s,s,s,a,{.2f,1,1});}
            for(int j=0;j<24;j++){float a=j*.2618f,r=.8f+effect*7;box(hitX+std::sin(a)*r,.16f,hitZ+std::cos(a)*r,.3f*(1-k),.04f,.14f,a,{.45f,1,1});}}
        float boost=extra()/12;if(boost>.01f)for(int i=0;i<7;i++){float d=1.3f+i*.45f,s=(1-i/7.f)*boost;box(carX-std::sin(yaw)*d,.35f,carZ-std::cos(yaw)*d,.5f*s,.35f*s,.65f,yaw,{.08f,.65f,1});}
    }
private:
    std::mt19937 rng{std::random_device{}()};
    void spawn(int i){std::uniform_real_distribution<float> jitter(.15f,.85f),lane(-2.7f,2.7f);float a=(i+jitter(rng))*6.283185f/8,r=lane(rng);bottles[i]={std::cos(a)*(36+r),std::sin(a)*(24+r),0};}
};
