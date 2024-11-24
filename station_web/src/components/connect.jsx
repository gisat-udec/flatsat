export default function () {
    let port;

    const connect = () => {
        port.connect().then(() => {
            port.onReceive = (data) => {
                let textDecoder = new TextDecoder();
                console.log(textDecoder.decode(data));
            };
            port.onReceiveError = (error) => {
                console.error(error);
            };
        });
    };

    const btnClick = () => {
        serial.requestPort().then((selectedPort) => {
            port = selectedPort;
            connect();
        });
    };
    return (
        <button class="btn btn-primary" onClick={btnClick}>
            hola
        </button>
    );
}
