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
        console.log("Connected");
        // read loop
        await Serial.Listen();
        // read loop broke
        Serial.Disconnect();
    },
    Read: async (len: number): Promise<DataView> => {
        if (port == undefined) throw new Error("Not connected");
        let i = 0;
        let buf = new ArrayBuffer(len);
        let uint8_buf_view = new Uint8Array(buf);
        while (i < len) {
            let result = await port.device.transferIn(
                port.endpoint_in,
                len - i,
            );
            if (result.data == undefined) throw new Error("Failed to read");
            uint8_buf_view.set(new Uint8Array(result.data.buffer), i);
            i += result.data.byteLength;
        }
        return new DataView(buf);
    },
    FindString: async (str: string, steps: number): Promise<boolean> => {
        let chars = Array.from(str).map((letter) => letter.charCodeAt(0));
        let pos = 0;
        for (let i = 0; i < steps + str.length; i++) {
            let byte = await Serial.Read(1);
            if (byte.getUint8(0) == chars[pos]) {
                pos += 1;
                if (pos == chars.length) {
                    return true;
                }
            } else {
                pos = 0;
            }
        }
        return false;
    },
    Listen: async () => {
        while (1) {
            // read header
            try {
                if (await Serial.FindString("start", 1600)) {
                    // read payload length
                    let len = (await Serial.Read(4)).getInt32(0, true);
                    // read payload
                    let data = await Serial.Read(len);
                    // read footer
                    if (await Serial.FindString("end", 0)) {
                        Serial.OnPacket(data);
                    } else {
                        console.error("Footer not found");
                    }
                }
            } catch (e) {
                console.error(e);
                break;
            }
        }
    },
    Disconnect: async () => {
        if (port == undefined) {
            return;
        }
        try {
            await port.device.controlTransferOut({
                requestType: "class",
                recipient: "interface",
                request: 0x22,
                value: 0x00,
                index: port.if_number,
            });
            await port.device.close();
        } finally {
            port = undefined;
            console.log("Disconnected");
        }
    },
    // callbacks
    OnPacket: (data: DataView) => {},
};

export default Serial;
