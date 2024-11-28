import { clients_signal } from "../scripts/app.ts";
import { For } from "solid-js";
import { createEffect } from "solid-js";

export default function () {
    createEffect(() => {
        // Access the current value of the signal
        const clients = clients_signal();

        // Loop through the array
        Object.values(clients).forEach((entry) => {
            console.log(entry);
        });
    });
    return (
        <div>
            <For each={Object.values(clients_signal())}>
                {(entry) => (
                    <div>
                        <div>accel x: {entry.imu_x}</div>
                        <div>accel y: {entry.imu_y}</div>
                        <div>accel z: {entry.imu_z}</div>
                        <div>temp: {entry.ambient_temp}</div>
                        <div>pressure: {entry.ambient_pressure}</div>
                        <img
                            src={entry.jpeg}
                            style="transform: scaleY(-1)"
                        ></img>
                        <br></br>
                    </div>
                )}
            </For>
        </div>
    );
}
