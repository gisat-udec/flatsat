import Serial from "../scripts/serial.ts";

export default function () {
    return (
        <button class="btn btn-primary" onClick={Serial.Connect}>
            Conectar
        </button>
    );
}
