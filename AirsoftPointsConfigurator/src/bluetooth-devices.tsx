import React, { Component } from "react";
import {
    Text, ScrollView, View, Button, StyleSheet,
} from "react-native";
import BluetoothService, { IDeviceListItem } from "./bluetooth-service";

export interface IBluetoothDevicesProps {
    onDeviceSelected: (device: IDeviceListItem) => void;
    bluetoothService: BluetoothService;
}

export interface IBluetoothDevicesState {
    deviceList: IDeviceListItem[];
    error?: string;
}

export default class BluetoothDevices extends Component<IBluetoothDevicesProps, IBluetoothDevicesState> {

    public componentDidMount() {
        this.updateView();
    }

    public render() {
        if (this.state && this.state.deviceList) {
            return (
                <>
                    <Button
                        title="Discover"
                        onPress={() => {
                            this.updateView();
                        }}
                    />
                    <ScrollView>
                        {this.state.deviceList.map((device: IDeviceListItem, index: number) => {
                            return (
                                <View
                                    style={styles.view}
                                    key={device.id}
                                >
                                    <Text style={styles.label}>{device.name}</Text>
                                    <Text style={styles.label}>{device.id}</Text>
                                    <View style={styles.label}>
                                        <Button
                                            title="Select"
                                            onPress={async () => {
                                                try {
                                                    this.props.onDeviceSelected(device);
                                                } catch (ex) {
                                                    this.setState({
                                                        error: ex.message,
                                                    });
                                                }
                                            }}
                                        ></Button>
                                    </View>
                                </View>
                            )
                        })
                        }
                    </ScrollView>
                </>);
        }
        if (this.state && this.state.error) {
            return <>
                <Text>{this.state.error}</Text>
            </>;
        }
        return null;
    }

    public async updateView() {
        try {
            const devices: IDeviceListItem[] = await this.props.bluetoothService.list();
            this.setState({
                deviceList: devices,
            });
        } catch (ex) {
            this.setState({
                error: ex.message,
            });
        }
    }
}

const styles = StyleSheet.create({
    view: {
        flexDirection: "row",
        borderWidth: 3,
        borderColor: "skyblue",
        margin: 3,
    },
    button: {
        margin: 1,
    },
    label: {
        textAlignVertical: "center",
        flex: 1,
        margin: 2,
    },
    input: {
        borderWidth: 2,
        flex: 3,
        margin: 1,
    },
});