let r;const o={Connect:async()=>{let e=await navigator.usb.requestDevice({filters:[{vendorId:9114}]});await e.open(),await e.selectConfiguration(1);let t,i=0,n=0;e.configuration?.interfaces.forEach(async a=>{let l=a.alternate;l.interfaceClass==255&&(t=a.interfaceNumber,l.endpoints.forEach(c=>{c.direction=="in"?i=c.endpointNumber:c.direction=="out"&&(n=c.endpointNumber)}))}),typeof t=="number"&&(await e.claimInterface(t),await e.selectAlternateInterface(t,0),await e.controlTransferOut({requestType:"class",recipient:"interface",request:34,value:1,index:t}),r={device:e,if_number:t,endpoint_in:i,endpoint_out:n},console.log("Connected"),await o.Listen(),o.Disconnect())},Read:async e=>{if(r==null)throw new Error("Not connected");let t=0,i=new ArrayBuffer(e),n=new Uint8Array(i);for(;t<e;){let a=await r.device.transferIn(r.endpoint_in,e-t);if(a.data==null)throw new Error("Failed to read");n.set(new Uint8Array(a.data.buffer),t),t+=a.data.byteLength}return new DataView(i)},FindString:async(e,t)=>{let i=Array.from(e).map(a=>a.charCodeAt(0)),n=0;for(let a=0;a<t+e.length;a++)if((await o.Read(1)).getUint8(0)==i[n]){if(n+=1,n==i.length)return!0}else n=0;return!1},Listen:async()=>{for(;;)try{if(await o.FindString("start",2e3)){let e=(await o.Read(4)).getInt32(0,!0),t=await o.Read(e-4);await o.FindString("end",0)?o.OnPacket(t):console.error("Footer not found")}}catch(e){console.error(e);break}},Disconnect:async()=>{if(r!=null)try{await r.device.controlTransferOut({requestType:"class",recipient:"interface",request:34,value:0,index:r.if_number}),await r.device.close()}finally{r=void 0,console.log("Disconnected")}},OnPacket:e=>{}};export{o as S};
