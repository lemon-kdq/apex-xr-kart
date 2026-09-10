import * as THREE from 'three';

// Platform-independent driving update; no DOM, camera, or renderer.
export function stepVehicle(state,input,dt){
let {x,z,speed,heading,steer}=state;const {gas,brake,handbrake}=input;
const norm=Math.hypot(x/36,z/24);const onTrack=Math.abs(norm-1)<.145;if(gas)speed+=10*dt;else if(brake)speed-=(speed>0?18:5)*dt;else speed=THREE.MathUtils.damp(speed,0,onTrack?.35:2,dt);if(handbrake)speed=THREE.MathUtils.damp(speed,0,7,dt);speed=THREE.MathUtils.clamp(speed,-4,onTrack?19:6);heading+=steer*speed*.055*dt;x+=Math.sin(heading)*speed*dt;z+=Math.cos(heading)*speed*dt;
const boundary=Math.hypot(x/(36+6),z/(24+6));if(boundary>1){x/=boundary;z/=boundary;speed*=-.2;}

return {x,z,speed,heading,steer};
}
