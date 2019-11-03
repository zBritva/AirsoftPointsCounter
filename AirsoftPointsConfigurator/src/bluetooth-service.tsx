import BluetoothSerial, { Buffer, AndroidBluetoothDevice, iOSBluetoothDevice } 
from "react-native-bluetooth-serial-next";
import { EmitterSubscription } from "react-native";

export interface IDeviceListItem {
    id: string;
    name: string;
}

export default class BluetoothService {
    public on(eventName: string, handler: (message: {}|undefined) => void): void {
        BluetoothSerial.on(eventName, handler as any);
    }
    public removeListener(eventName: string, handler: () => void): void {
        BluetoothSerial.removeListener(eventName, handler);
    }
    public write(data: Buffer | string): Promise<boolean> {
        return BluetoothSerial.write(data);
    }
    public list(): Promise<IDeviceListItem[]> {
        return BluetoothSerial.list();
    }
    public isEnabled(): Promise<boolean> {
        return BluetoothSerial.isEnabled();
    }
    public connect(id: string): Promise<AndroidBluetoothDevice | iOSBluetoothDevice> {
        return BluetoothSerial.connect(id);
    }
    public disconnect(): Promise<boolean> {
        return BluetoothSerial.disconnect();
    }
    public isConnected(id: string): Promise<boolean> {
        return BluetoothSerial.isConnected(id);
    }
    public async readBuffer(id?: string): Promise<string> {
        return BluetoothSerial.readFromDevice(id);
    }
    public onData(id: string, handler: (data: string, subscription: EmitterSubscription) => {}, delimiter?: ""): void {
        return BluetoothSerial.read(handler, delimiter, id);
    }
}
