import * as THREE from 'three';
import fs from 'node:fs';
const source=fs.readFileSync(new URL('./src/main.js',import.meta.url),'utf8');
let build=source.slice(source.indexOf('const mat=c=>'),source.indexOf('const keys=new Set'));
const start=build.indexOf('const sign=document');
const end=build.indexOf('for(let i=0;i<4;i++)',start);
build=build.slice(0,start)+build.slice(end);
build=build.replace('new THREE.PlaneGeometry(500,500)','new THREE.PlaneGeometry(158,118)');
const scene=new THREE.Scene();
const kart=new Function('THREE','scene',build+';return kart;')(THREE,scene);
// Geometry text remains readable without an Android font or texture dependency.
const glyph={A:['010','101','111','101','101'],P:['110','101','110','100','100'],E:['111','100','110','100','111'],X:['101','101','010','101','101'],S:['111','100','111','001','111'],T:['111','010','010','010','010'],R:['110','101','110','101','101']};
const label='APEX START';let cursor=30.9;
for(const c of label){if(glyph[c])glyph[c].forEach((row,y)=>[...row].forEach((v,x)=>{if(v==='1'){const o=new THREE.Mesh(new THREE.BoxGeometry(.18,.14,.02),new THREE.MeshBasicMaterial({color:'#fff3dc'}));o.position.set(cursor+x*.22,8.35-y*.17,.19);scene.add(o);}}));cursor+=1.15;}
// Soft-looking stepped ground contact patches, geometry only.
const shadows=[];scene.traverse(o=>{if(o.isMesh&&o.geometry.type==='CylinderGeometry'&&o.position.y>1)shadows.push([o.position.x,o.position.z,2.5]);});
for(const [x,z,r] of shadows){const o=new THREE.Mesh(new THREE.CircleGeometry(r,12),new THREE.MeshBasicMaterial({color:'#639348'}));o.rotation.x=-Math.PI/2;o.position.set(x,.012,z);scene.add(o);}
scene.updateMatrixWorld(true);
const sun=new THREE.Vector3(-.4,.85,.35).normalize();
function vertices(root,skip){const result=[];root.traverse(o=>{if(!o.isMesh||skip?.(o))return;const g=o.geometry.index?o.geometry.toNonIndexed():o.geometry;const pos=g.attributes.position,norm=g.attributes.normal;const nm=new THREE.Matrix3().getNormalMatrix(o.matrixWorld);const base=o.material.color;for(let i=0;i<pos.count;i++){const p=new THREE.Vector3().fromBufferAttribute(pos,i).applyMatrix4(o.matrixWorld);const n=new THREE.Vector3().fromBufferAttribute(norm,i).applyMatrix3(nm).normalize();const light=o.material.isMeshBasicMaterial?1:.57+.43*Math.max(0,n.dot(sun));result.push(...p.toArray(),base.r*light,base.g*light,base.b*light);}});return result;}
function isKart(o){while(o){if(o===kart)return true;o=o.parent;}return false;}
const world=vertices(scene,isKart),car=vertices(kart);
const header='#pragma once\n// Generated from kart/src/main.js by kart/export-xr.mjs.\n'+[ ['World',world],['Car',car] ].map(([name,v])=>'static const float k'+name+'Vertices[]={\n'+v.map(n=>Number(n.toFixed(5)).toString()+'f').map(s=>s.includes('.')?s:s.replace('f','.0f')).join(',')+'\n};\n').join('');
fs.writeFileSync(new URL('../xr-test-sdk/src/tests/hello_xr/kart_assets.h',import.meta.url),header);
console.log(JSON.stringify({worldVertices:world.length/6,carVertices:car.length/6,bytes:header.length}));
