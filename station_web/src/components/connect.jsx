export default function () {
    const btnClick = () => {
        serial.requestPort().then((selectedPort) => {
            port = selectedPort;
            //connect();
        });
    };
    return (
        <button class="btn btn-primary" onClick={btnClick}>
            hola
        </button>
    );
}
