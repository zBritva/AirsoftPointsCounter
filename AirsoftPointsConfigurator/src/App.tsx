import React, { Component } from "react";
import {
  StatusBar,
} from "react-native";

import BluetoothDevicesManager from "./bluetooth-devices-manager";
import BluetoothService from "./bluetooth-service"

export default class App extends Component {
  private bluetoothService: BluetoothService;
  constructor(props: any) {
    super(props);
    this.bluetoothService = new BluetoothService();
  }

  render() {
    return (
      <>
        <StatusBar></StatusBar>
        <BluetoothDevicesManager bluetoothService = {this.bluetoothService}>

        </BluetoothDevicesManager>
      </>
    );
  };
}
