import React, { Component } from "react";
import {
    Text, Button, EmitterSubscription, TextInput, View, Alert, StyleSheet,
} from "react-native";
import BluetoothService, { IDeviceListItem } from "./bluetooth-service";

export interface IDeviceConfiguratorProps {
    bluetoothService: BluetoothService;
    device: IDeviceListItem;
    onDisconnect: () => void;
}

export interface IDeviceConfiguratorState {
    properties?: IDeviceProperties;
    ediatableProperties?: IDeviceProperties;
    isConnected: boolean;
    error?: string;
}

export interface IDeviceProperties {
    [key: string]: string;
}

export default class DeviceConfigurator extends Component<IDeviceConfiguratorProps, IDeviceConfiguratorState> {

    constructor(props: IDeviceConfiguratorProps) {
        super(props);
        this.state = {
            error: undefined,
            isConnected: false,
            properties: undefined,
            ediatableProperties: undefined,
        };
    }

    public async componentDidMount() {
        const { bluetoothService } = this.props;
        bluetoothService.on("connectionSuccess", () => {
            this.setState({
                isConnected: true,
            });
            this.getSettings();
        });
        bluetoothService.on("connectionLost", () => {
            this.setState({
                isConnected: false,
            });
        });
        await this.connectToDevice();
        await this.getSettings();
    }

    public parseSettings(settings: string): IDeviceProperties {
        try {
            const propertyList = settings.split("\n");
            const properties: IDeviceProperties = {};
            propertyList.forEach((property: string) => {
                const [key, value] = property.split(" ");
                properties[key] = value;
            });

            if (Object.keys(properties).length === 7) {
                return properties;
            } else {
                return null;
            }
        } catch (ex) {
            this.setState({
                error: ex.messages,
            });
            return null;
        }
    }

    public render() {
        const { device } = this.props;
        return (
            <>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >{device.name} status: </Text>
                    <Text style={styles.label} >{this.state.isConnected ? "Connected" : "Disconnected"}</Text>
                </View>
                {/* <Text>{JSON.stringify(this.state.properties, null, " ")}</Text> */}
                <Text>{this.state.error ? this.state.error : null}</Text>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Hold time</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="Time to hold for point (sec.)"
                        onChangeText={(text) => {
                            this.updateProperty("-t", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-t"]}
                    />
                </View>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Point per time</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="Count of point per hold time"
                        onChangeText={(text) => {
                            this.updateProperty("-p", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-p"]}
                    />
                </View>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Capture time</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="TIme for capture the point (sec.)"
                        onChangeText={(text) => {
                            this.updateProperty("-c", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-c"]}
                    />
                </View>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Points limit</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="Limit of point to win the game"
                        onChangeText={(text) => {
                            this.updateProperty("-l", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-l"]}
                    />
                </View>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Display 1</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="Points display time"
                        onChangeText={(text) => {
                            this.updateProperty("-it0", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-it0"]}
                    />
                </View>
                <View style={{ flexDirection: "row" }}>
                    <Text style={styles.label} >Display 2</Text>
                    <TextInput
                        style={styles.input}
                        placeholder="Time to point display time"
                        onChangeText={(text) => {
                            this.updateProperty("-it1", text);
                        }}
                        value={this.state.ediatableProperties && this.state.ediatableProperties["-it1"]}
                    />
                </View>
                {
                    this.state.isConnected ?
                        <>
                            <View style={{ margin: 3 }}>
                                <Button
                                    title="Save"
                                    onPress={() => {
                                        this.saveSettings();
                                    }}
                                />
                            </View>
                            <View style={{ margin: 3 }}>
                                <Button
                                    title="Get settings"
                                    onPress={() => {
                                        this.getSettings();
                                    }}
                                />
                            </View>
                            <View style={{ margin: 3 }}>
                                <Button
                                    title="Restart"
                                    onPress={() => {
                                        // Works on both iOS and Android
                                        Alert.alert(
                                            "Restarting",
                                            "Restart the game?",
                                            [
                                                {
                                                    text: "Cancel",
                                                    // tslint:disable-next-line: object-literal-sort-keys
                                                    style: "cancel",
                                                },
                                                { text: "OK", onPress: () => this.restart() },
                                            ],
                                            { cancelable: false },
                                        );
                                    }}
                                />
                            </View>
                        </>
                        :
                        <>
                            <View style={{ margin: 3 }}>
                                <Button
                                    title="Connect"
                                    onPress={() => {
                                        this.connectToDevice();
                                    }}
                                />
                            </View>
                        </>
                }

                <View style={{ margin: 3 }}>
                    <Button
                        title="Disconnect/ unselect"
                        onPress={() => {
                            this.onDisconnect();
                        }}
                    />
                </View>
            </>
        );
    }

    private async updateProperty(key: string, value: string) {
        try {
            this.setState((prev: IDeviceConfiguratorState) => {
                return {
                    ediatableProperties: {
                        ...prev.ediatableProperties,
                        [key]: value,
                    },
                };
            });
        } catch (ex) {
            this.setState({
                error: ex.message,
            });
        }
    }

    private async getSettings() {
        try {
            await this.props.bluetoothService.write("-s");
            const data = await this.props.bluetoothService.readBuffer(this.props.device.id);
            const savedProperties = this.parseSettings(data);
            if (savedProperties) {
                this.setState({
                    properties: savedProperties,
                    ediatableProperties: savedProperties
                });
            }
        } catch (ex) {
            this.setState({
                error: ex.message,
            });
        }
    }

    private async restart() {
        try {
            await this.props.bluetoothService.write("-r");
        } catch (ex) {
            this.setState({
                error: ex.message,
            });
        }
    }

    private async onDisconnect() {
        await this.props.bluetoothService.disconnect();
        this.props.onDisconnect();
    }

    private async saveSettings() {
        if (!this.state.properties) {
            return;
        }
        for (const property of Object.keys(this.state.properties)) {
            if (this.state.properties && this.state.ediatableProperties) {
                if (this.state.ediatableProperties[property] !== this.state.properties[property]) {
                    await this.setProperty(property, this.state.ediatableProperties[property]);
                    await this.waitSec();
                }
            }
        }
        this.getSettings();
    }

    private async waitSec() {
        return new Promise<void>((resolve) => {
            setTimeout(() => {
                resolve();
            }, 1000);
        });
    }

    private async setProperty(key: string, value: string) {
        try {
            if (!value) {
                return;
            }
            await this.props.bluetoothService.write(`${key} ${value}}`);
            this.setState({
                error: `${key} ${value}`,
            });
        } catch (ex) {
            this.setState({
                error: ex.message,
            });
        }
    }

    private async connectToDevice() {
        try {
            if (! await this.props.bluetoothService.isConnected(this.props.device.id)) {
                await this.props.bluetoothService.connect(this.props.device.id);
            } else {
                this.setState({
                    isConnected: true,
                });
            }
        } catch (ex) {
            this.setState({
                error: ex.message,
                isConnected: false,
            });
        }
    }
}

const styles = StyleSheet.create({
    bigBlue: {
        color: 'blue',
        fontWeight: 'bold',
        fontSize: 30,
    },
    button: {
        margin: 1,
    },
    label: {
        textAlignVertical: "center",
        flex: 1,
        marginLeft: 3,
    },
    input: {
        borderWidth: 2,
        flex: 3,
        margin: 1,
        borderColor: "skyblue",
    },
});