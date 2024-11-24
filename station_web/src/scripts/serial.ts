import App from "./app.ts";

interface Port {
    device: USBDevice;
    if_number: number;
    endpoint_in: number;
    endpoint_out: number;
}

let port: Port | undefined;

const Serial = {
    Connect: async () => {
        let device = await navigator.usb.requestDevice({
            filters: [{ vendorId: 0x239a }],
        });
        await device.open();
        await device.selectConfiguration(1);
        let if_number: number | undefined;
        let endpoint_in: number = 0;
        let endpoint_out: number = 0;
        device.configuration?.interfaces.forEach(async (intf) => {
            let alt = intf.alternate;
            if (alt.interfaceClass == 0xff) {
                if_number = intf.interfaceNumber;
                alt.endpoints.forEach((endpoint) => {
                    if (endpoint.direction == "in") {
                        endpoint_in = endpoint.endpointNumber;
                    } else if (endpoint.direction == "out") {
                        endpoint_out = endpoint.endpointNumber;
                    }
                });
            }
        });
        if (typeof if_number != "number") {
            return;
        }
        await device.claimInterface(if_number);
        await device.selectAlternateInterface(if_number, 0);
        await device.controlTransferOut({
            requestType: "class",
            recipient: "interface",
            request: 0x22,
            value: 0x01,
            index: if_number,
        });
        // save to scope
        port = {
            device: device,
            if_number: if_number,
            endpoint_in: endpoint_in,
            endpoint_out: endpoint_out,
        };
        // read loop
        await Serial.Listen();
        // read loop broke
        port = undefined;
    },
    ReadChunk: async (): Promise<DataView | undefined> => {
        if (port == undefined) return undefined;
        try {
            let result = await port.device.transferIn(port.endpoint_in, 1600);
            if (result.data == undefined) return undefined;
            return result.data;
        } catch (e) {
            console.error(e);
            return undefined;
        }
    },
    FindString: (data: DataView, str: string): number | undefined => {
        let chars = Array.from(str).map((letter) => letter.charCodeAt(0));
        let pos = 0;
        for (let i = 0; i < data.byteLength; i++) {
            if (data.getUint8(i) == chars[pos]) {
                pos = pos + 1;
                if (pos == str.length) {
                    return i;
                }
            } else {
                pos = 0;
            }
        }
        return undefined;
    },
    Listen: async () => {
        let reading = false;
        let pos = 0;
        let buf = new Uint8Array(3000);
        while (1) {
            let data = await Serial.ReadChunk();
            if (data == undefined) return;
            let header_end = Serial.FindString(data, "start");
            if (typeof header_end == "number") {
                reading = true;
                if (data.byteLength - header_end >= 4) {
                    console.log(
                        data.getUint8(header_end + 1),
                        data.getUint8(header_end + 2),
                        data.getUint8(header_end + 3),
                        data.getUint8(header_end + 4),
                        data.getInt32(header_end + 1, true),
                    );
                }
                console.log(header_end, data.byteLength);
            }
        }
    },
    Disconnect: async () => {
        if (port == undefined) {
            return;
        }
        await port.device.controlTransferOut({
            requestType: "class",
            recipient: "interface",
            request: 0x22,
            value: 0x00,
            index: port.if_number,
        });
        await port.device.close();
        port = undefined;
    },
    OnData: (data: DataView) => {
        console.log(data);
    },
};

export default Serial;
