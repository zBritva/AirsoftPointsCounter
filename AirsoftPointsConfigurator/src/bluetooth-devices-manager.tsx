import React, { Component } from "react";
import {
  Text, View, Button, Linking
} from "react-native";
import BluetoothDevices from "./bluetooth-devices";
import BluetoothService, { IDeviceListItem } from "./bluetooth-service";
import DeviceConfigurator from "./device-configurator";

export interface IBluetoothDevicesManagerProps {
  bluetoothService: BluetoothService;
}

export interface IBluetoothDevicesManagerState {
  currentDevice?: IDeviceListItem;
}

export default class BluetoothDevicesManager
  extends Component<IBluetoothDevicesManagerProps, IBluetoothDevicesManagerState> {

  constructor(props: IBluetoothDevicesManagerProps) {
    super(props);
    this.state = {
      currentDevice: undefined,
    };
  }

  public componentDidMount() {
    this.setState({
      currentDevice: undefined,
    });
  }

  public render() {
    return (
      <>
        {
          !this.state.currentDevice ?
          <>
            <BluetoothDevices
              onDeviceSelected={(device) => this.onDeviceSelected(device)}
              bluetoothService={this.props.bluetoothService}
            />
            <View>
              <Text>
                The open source software for setting properties for adruino hardware throw Bluetooth.
                You can build own hardware with arduino and use published firmware
              </Text>
              <Text>
                MIT License
              </Text>
              <Button
                title="Open sources"
                onPress={
                  () => {
                    Linking.openURL("https://github.com/zBritva/AirsoftPointsCounter");
                  }
                }
              />
            </View>
          </>
            : null
        }
        {
          this.state.currentDevice ?
            <DeviceConfigurator
              onDisconnect={() => this.onDisconnect()}
              device={this.state.currentDevice}
              bluetoothService={this.props.bluetoothService}
            /> : null}
      </>
    );
  }

  private onDisconnect() {
    this.setState({
      currentDevice: undefined,
    });
  }

  private onDeviceSelected(device: IDeviceListItem) {
    // tslint:disable-next-line
    this.setState((previous: IBluetoothDevicesManagerState) => {
      if (previous.currentDevice !== device) {
        this.setState({
          currentDevice: device,
        });
      }
    });
  }
}
