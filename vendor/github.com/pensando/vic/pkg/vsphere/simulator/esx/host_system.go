// Copyright 2016 VMware, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package esx

import (
	"fmt"
	"math/rand"
	"time"

	"github.com/vmware/govmomi/vim25/mo"
	"github.com/vmware/govmomi/vim25/types"
)

var DvsUuid string
var hostList []*mo.HostSystem
var HostSystem = mo.HostSystem{
	ManagedEntity: mo.ManagedEntity{
		ExtensibleManagedObject: mo.ExtensibleManagedObject{
			Self:           types.ManagedObjectReference{Type: "HostSystem", Value: "ha-host"},
			Value:          nil,
			AvailableField: nil,
		},
		Parent:              &types.ManagedObjectReference{Type: "ComputeResource", Value: "ha-compute-res"},
		CustomValue:         nil,
		OverallStatus:       "",
		ConfigStatus:        "",
		ConfigIssue:         nil,
		EffectiveRole:       nil,
		Permission:          nil,
		Name:                "",
		DisabledMethod:      nil,
		RecentTask:          nil,
		DeclaredAlarmState:  nil,
		TriggeredAlarmState: nil,
		AlarmActionsEnabled: (*bool)(nil),
		Tag:                 nil,
	},
	Runtime: types.HostRuntimeInfo{
		DynamicData:       types.DynamicData{},
		ConnectionState:   "connected",
		PowerState:        "poweredOn",
		StandbyMode:       "",
		InMaintenanceMode: false,
		BootTime:          (*time.Time)(nil),
		HealthSystemRuntime: &types.HealthSystemRuntime{
			DynamicData: types.DynamicData{},
			SystemHealthInfo: &types.HostSystemHealthInfo{
				DynamicData: types.DynamicData{},
				NumericSensorInfo: []types.HostNumericSensorInfo{
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware Rollup Health State",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "system",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #0 Level-1 Cache is 16384 B",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Processors",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #0 Level-2 Cache is 0 B",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Processors",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #1 Level-1 Cache is 16384 B",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Processors",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #1 Level-2 Cache is 0 B",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Processors",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "Phoenix Technologies LTD System BIOS 6.00 2014-05-20 00:00:00.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware, Inc. VMware ESXi 6.0.0 build-3634798 2016-03-07 00:00:00.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-ata-piix 2.12-10vmw.600.2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-lsi-mptsas-plugin 1.0.0-1vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-mlx4-core 1.9.7.0-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-lsi-mpt2sas-plugin 1.0.0-4vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-aacraid 1.1.5.1-9vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-via 0.3.3-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-qla4xxx 5.01.03.2-7vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-sata-promise 2.12-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-megaraid-mbox 2.20.5.1-6vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware vsan 6.0.0-2.34.3563498 2016-02-17 17:18:19.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-e1000 8.0.3.1-5vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-serverworks 0.4.3-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-mptspi 4.23.01.00-9vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-nx-nic 5.0.621-5vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware block-cciss 3.6.14-10vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-bnx2x 1.78.80.v60.12-1vmw.600.2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ipmi-ipmi-devintf 39.1-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-mptsas 4.23.01.00-9vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-megaraid2 2.00.4-9vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware nvme 1.0e.0.35-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware esx-xserver 6.0.0-2.34.3634798 2016-03-08 07:39:27.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware nmlx4-en 3.0.0.0-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-hp-hpsa-plugin 1.0.0-1vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-megaraid-sas 6.603.55.00-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-enic 2.1.2.38-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsi-msgpt3 06.255.12.00-8vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-ahci 3.0-22vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-forcedeth 0.61-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-atiixp 0.4.6-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware elxnet 10.2.309.6v-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware esx-dvfilter-generic-fastpath 6.0.0-2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware uhci-usb-uhci 1.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-amd 0.3.10-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-sata-sil24 1.1-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ohci-usb-ohci 1.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-igb 5.0.5.1.1-5vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-pdc2027x 1.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ehci-ehci-hcd 1.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-lsi-lsi-mr3-plugin 1.0.0-2vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-ixgbe 3.7.13.7.14iov-20vmw.600.2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware vsanhealth 6.0.0-3000000.3.0.2.34.3544323 2016-02-12 06:45:30.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-cnic 1.78.76.v60.13-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-sata-svw 2.3-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ipmi-ipmi-msghandler 39.1-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware emulex-esx-elxnetcli 10.2.309.6v-2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-aic79xx 3.1-5vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware qlnativefc 2.0.12.0-5vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-lsi-lsi-msgpt3-plugin 1.0.0-1vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ima-qla4xxx 2.02.18-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-mlx4-en 1.9.7.0-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-e1000e 3.2.2.1-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-tg3 3.131d.v60.4-2vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-hpsa 6.0.0.44-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-bnx2fc 1.78.78.v60.8-1vmw.600.2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware cpu-microcode 6.0.0-2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-fnic 1.5.0.45-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware nmlx4-rdma 3.0.0.0-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-vmxnet3 1.1.3.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lpfc 10.2.309.8-2vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware esx-ui 1.0.0-3617585 2016-03-03 04:52:43.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-cmd64x 0.2.5-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsi-mr3 6.605.08.00-7vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-hpt3x2n 0.3.4-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-sata-nv 3.5-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware misc-cnic-register 1.78.75.v60.7-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware lsu-lsi-megaraid-sas-plugin 1.0.0-2vmw.600.2.34.3634798 2016-03-08 07:39:28.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ata-pata-sil680 0.4.8-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware esx-tboot 6.0.0-2.34.3634798 2016-03-08 07:39:27.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware xhci-xhci 1.0-3vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-ips 7.12.05-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-adp94xx 1.0.8.12-6vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware rste 2.0.2.0088-4vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware ipmi-ipmi-si-drv 39.1-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMWARE mtip32xx-native 3.8.5-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-mpt2sas 19.00.00.00-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware misc-drivers 6.0.0-2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware nmlx4-core 3.0.0.0-1vmw.600.2.34.3634798 2016-03-08 07:38:46.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware sata-sata-sil 2.3-4vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware esx-base 6.0.0-2.34.3634798 2016-03-08 07:39:18.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware scsi-bnx2i 2.78.76.v60.8-1vmw.600.2.34.3634798 2016-03-08 07:38:41.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "VMware net-bnx2 2.2.4f.v60.10-1vmw.600.2.34.3634798 2016-03-08 07:38:45.000",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "e1000 driver 8.0.3.1-NAPI",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
					{
						DynamicData: types.DynamicData{},
						Name:        "e1000 device firmware N/A",
						HealthState: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Sensor is operating under normal conditions",
							},
							Key: "green",
						},
						CurrentReading: 0,
						UnitModifier:   0,
						BaseUnits:      "",
						RateUnits:      "",
						SensorType:     "Software Components",
					},
				},
			},
			HardwareStatusInfo: &types.HostHardwareStatusInfo{
				DynamicData:      types.DynamicData{},
				MemoryStatusInfo: nil,
				CpuStatusInfo: []types.BaseHostHardwareElementInfo{
					&types.HostHardwareElementInfo{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #0",
						Status: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Physical element is functioning as expected",
							},
							Key: "Green",
						},
					},
					&types.HostHardwareElementInfo{
						DynamicData: types.DynamicData{},
						Name:        "CPU socket #1",
						Status: &types.ElementDescription{
							Description: types.Description{
								DynamicData: types.DynamicData{},
								Label:       "Green",
								Summary:     "Physical element is functioning as expected",
							},
							Key: "Green",
						},
					},
				},
				StorageStatusInfo: nil,
			},
		},
		DasHostState:    (*types.ClusterDasFdmHostState)(nil),
		TpmPcrValues:    nil,
		VsanRuntimeInfo: &types.VsanHostRuntimeInfo{},
		NetworkRuntimeInfo: &types.HostRuntimeInfoNetworkRuntimeInfo{
			DynamicData: types.DynamicData{},
			NetStackInstanceRuntimeInfo: []types.HostRuntimeInfoNetStackInstanceRuntimeInfo{
				{
					DynamicData:            types.DynamicData{},
					NetStackInstanceKey:    "defaultTcpipStack",
					State:                  "active",
					VmknicKeys:             []string{"vmk0"},
					MaxNumberOfConnections: 11000,
					CurrentIpV6Enabled:     types.NewBool(true),
				},
			},
			NetworkResourceRuntime: (*types.HostNetworkResourceRuntime)(nil),
		},
		VFlashResourceRuntimeInfo:  (*types.HostVFlashManagerVFlashResourceRunTimeInfo)(nil),
		HostMaxVirtualDiskCapacity: 68169720922112,
	},
	Summary: types.HostListSummary{
		DynamicData: types.DynamicData{},
		Host:        &types.ManagedObjectReference{Type: "HostSystem", Value: "ha-host"},
		Hardware: &types.HostHardwareSummary{
			DynamicData: types.DynamicData{},
			Vendor:      "VMware, Inc.",
			Model:       "VMware Virtual Platform",
			Uuid:        "564d2f12-8041-639b-5018-05a835b72eaf",
			OtherIdentifyingInfo: []types.HostSystemIdentificationInfo{
				{
					DynamicData:     types.DynamicData{},
					IdentifierValue: " No Asset Tag",
					IdentifierType: &types.ElementDescription{
						Description: types.Description{
							DynamicData: types.DynamicData{},
							Label:       "Asset Tag",
							Summary:     "Asset tag of the system",
						},
						Key: "AssetTag",
					},
				},
				{
					DynamicData:     types.DynamicData{},
					IdentifierValue: "[MS_VM_CERT/SHA1/27d66596a61c48dd3dc7216fd715126e33f59ae7]",
					IdentifierType: &types.ElementDescription{
						Description: types.Description{
							DynamicData: types.DynamicData{},
							Label:       "OEM specific string",
							Summary:     "OEM specific string",
						},
						Key: "OemSpecificString",
					},
				},
				{
					DynamicData:     types.DynamicData{},
					IdentifierValue: "Welcome to the Virtual Machine",
					IdentifierType: &types.ElementDescription{
						Description: types.Description{
							DynamicData: types.DynamicData{},
							Label:       "OEM specific string",
							Summary:     "OEM specific string",
						},
						Key: "OemSpecificString",
					},
				},
				{
					DynamicData:     types.DynamicData{},
					IdentifierValue: "VMware-56 4d 2f 12 80 41 63 9b-50 18 05 a8 35 b7 2e af",
					IdentifierType: &types.ElementDescription{
						Description: types.Description{
							DynamicData: types.DynamicData{},
							Label:       "Service tag",
							Summary:     "Service tag of the system",
						},
						Key: "ServiceTag",
					},
				},
			},
			MemorySize:    4294430720,
			CpuModel:      "Intel(R) Core(TM) i7-3615QM CPU @ 2.30GHz",
			CpuMhz:        2294,
			NumCpuPkgs:    2,
			NumCpuCores:   2,
			NumCpuThreads: 2,
			NumNics:       1,
			NumHBAs:       3,
		},
		Runtime: (*types.HostRuntimeInfo)(nil),
		Config: types.HostConfigSummary{
			DynamicData:   types.DynamicData{},
			Name:          "localhost.localdomain",
			Port:          902,
			SslThumbprint: "",
			Product: &types.AboutInfo{
				DynamicData:           types.DynamicData{},
				Name:                  "VMware ESXi",
				FullName:              "VMware ESXi 6.0.0 build-3634798",
				Vendor:                "VMware, Inc.",
				Version:               "6.0.0",
				Build:                 "3634798",
				LocaleVersion:         "INTL",
				LocaleBuild:           "000",
				OsType:                "vmnix-x86",
				ProductLineId:         "embeddedEsx",
				ApiType:               "HostAgent",
				ApiVersion:            "6.0",
				InstanceUuid:          "",
				LicenseProductName:    "VMware ESX Server",
				LicenseProductVersion: "6.0",
			},
			VmotionEnabled:        false,
			FaultToleranceEnabled: types.NewBool(true),
			FeatureVersion: []types.HostFeatureVersionInfo{
				{
					DynamicData: types.DynamicData{},
					Key:         "faultTolerance",
					Value:       "6.0.0-6.0.0-6.0.0",
				},
			},
			AgentVmDatastore: (*types.ManagedObjectReference)(nil),
			AgentVmNetwork:   (*types.ManagedObjectReference)(nil),
		},
		QuickStats: types.HostListSummaryQuickStats{
			DynamicData:               types.DynamicData{},
			OverallCpuUsage:           67,
			OverallMemoryUsage:        1404,
			DistributedCpuFairness:    0,
			DistributedMemoryFairness: 0,
			Uptime: 77229,
		},
		OverallStatus:      "gray",
		RebootRequired:     false,
		CustomValue:        nil,
		ManagementServerIp: "",
		MaxEVCModeKey:      "",
		CurrentEVCModeKey:  "",
		Gateway:            (*types.HostListSummaryGatewaySummary)(nil),
	},
	Hardware:           (*types.HostHardwareInfo)(nil),
	Capability:         (*types.HostCapability)(nil),
	LicensableResource: types.HostLicensableResourceInfo{},
	ConfigManager: types.HostConfigManager{
		DynamicData:               types.DynamicData{},
		CpuScheduler:              &types.ManagedObjectReference{Type: "HostCpuSchedulerSystem", Value: "cpuSchedulerSystem"},
		DatastoreSystem:           &types.ManagedObjectReference{Type: "HostDatastoreSystem", Value: "ha-datastoresystem"},
		MemoryManager:             &types.ManagedObjectReference{Type: "HostMemorySystem", Value: "memoryManagerSystem"},
		StorageSystem:             &types.ManagedObjectReference{Type: "HostStorageSystem", Value: "storageSystem"},
		NetworkSystem:             &types.ManagedObjectReference{Type: "HostNetworkSystem", Value: "networkSystem"},
		VmotionSystem:             &types.ManagedObjectReference{Type: "HostVMotionSystem", Value: "ha-vmotion-system"},
		VirtualNicManager:         &types.ManagedObjectReference{Type: "HostVirtualNicManager", Value: "ha-vnic-mgr"},
		ServiceSystem:             &types.ManagedObjectReference{Type: "HostServiceSystem", Value: "serviceSystem"},
		FirewallSystem:            &types.ManagedObjectReference{Type: "HostFirewallSystem", Value: "firewallSystem"},
		AdvancedOption:            &types.ManagedObjectReference{Type: "OptionManager", Value: "ha-adv-options"},
		DiagnosticSystem:          &types.ManagedObjectReference{Type: "HostDiagnosticSystem", Value: "diagnosticsystem"},
		AutoStartManager:          &types.ManagedObjectReference{Type: "HostAutoStartManager", Value: "ha-autostart-mgr"},
		SnmpSystem:                &types.ManagedObjectReference{Type: "HostSnmpSystem", Value: "ha-snmp-agent"},
		DateTimeSystem:            &types.ManagedObjectReference{Type: "HostDateTimeSystem", Value: "dateTimeSystem"},
		PatchManager:              &types.ManagedObjectReference{Type: "HostPatchManager", Value: "ha-host-patch-manager"},
		ImageConfigManager:        &types.ManagedObjectReference{Type: "HostImageConfigManager", Value: "ha-image-config-manager"},
		BootDeviceSystem:          (*types.ManagedObjectReference)(nil),
		FirmwareSystem:            &types.ManagedObjectReference{Type: "HostFirmwareSystem", Value: "ha-firmwareSystem"},
		HealthStatusSystem:        &types.ManagedObjectReference{Type: "HostHealthStatusSystem", Value: "healthStatusSystem"},
		PciPassthruSystem:         &types.ManagedObjectReference{Type: "HostPciPassthruSystem", Value: "ha-pcipassthrusystem"},
		LicenseManager:            &types.ManagedObjectReference{Type: "LicenseManager", Value: "ha-license-manager"},
		KernelModuleSystem:        &types.ManagedObjectReference{Type: "HostKernelModuleSystem", Value: "kernelModuleSystem"},
		AuthenticationManager:     &types.ManagedObjectReference{Type: "HostAuthenticationManager", Value: "ha-auth-manager"},
		PowerSystem:               &types.ManagedObjectReference{Type: "HostPowerSystem", Value: "ha-power-system"},
		CacheConfigurationManager: &types.ManagedObjectReference{Type: "HostCacheConfigurationManager", Value: "ha-cache-configuration-manager"},
		EsxAgentHostManager:       (*types.ManagedObjectReference)(nil),
		IscsiManager:              &types.ManagedObjectReference{Type: "IscsiManager", Value: "iscsiManager"},
		VFlashManager:             &types.ManagedObjectReference{Type: "HostVFlashManager", Value: "ha-vflash-manager"},
		VsanSystem:                &types.ManagedObjectReference{Type: "HostVsanSystem", Value: "vsanSystem"},
		MessageBusProxy:           &types.ManagedObjectReference{Type: "MessageBusProxy", Value: "messageBusProxy"},
		UserDirectory:             &types.ManagedObjectReference{Type: "UserDirectory", Value: "ha-user-directory"},
		AccountManager:            &types.ManagedObjectReference{Type: "HostLocalAccountManager", Value: "ha-localacctmgr"},
		HostAccessManager:         &types.ManagedObjectReference{Type: "HostAccessManager", Value: "ha-host-access-manager"},
		GraphicsManager:           &types.ManagedObjectReference{Type: "HostGraphicsManager", Value: "ha-graphics-manager"},
		VsanInternalSystem:        &types.ManagedObjectReference{Type: "HostVsanInternalSystem", Value: "ha-vsan-internal-system"},
		CertificateManager:        &types.ManagedObjectReference{Type: "HostCertificateManager", Value: "ha-certificate-manager"},
	},
	Config:           (*types.HostConfigInfo)(nil),
	Vm:               nil,
	Datastore:        nil,
	Network:          nil,
	DatastoreBrowser: types.ManagedObjectReference{Type: "HostDatastoreBrowser", Value: "ha-host-datastorebrowser"},
	SystemResources:  (*types.HostSystemResourceInfo)(nil),
}

var pnicBacking = &types.DistributedVirtualSwitchHostMemberPnicBacking{
	PnicSpec: []types.DistributedVirtualSwitchHostMemberPnicSpec{{PnicDevice: "vmnic0"}},
}

func NewHostConfigInfo() *types.HostConfigInfo {
	// generate a mac for the Pnic
	mac := fmt.Sprintf("0c:c4:7a:%02x:%02x:%02x", rand.Intn(255), rand.Intn(255), rand.Intn(255))
	pnic := types.PhysicalNic{Key: "key-vim.host.PhysicalNic-vmnic0", Device: "vmnic0", Mac: mac}
	dvs := types.HostProxySwitch{DvsUuid: DvsUuid, Spec: types.HostProxySwitchSpec{Backing: types.BaseDistributedVirtualSwitchHostMemberBacking(pnicBacking)}}
	hni := types.HostNetworkInfo{Pnic: []types.PhysicalNic{pnic}, ProxySwitch: []types.HostProxySwitch{dvs}}
	hci := types.HostConfigInfo{Network: &hni}
	return &hci
}

func NewHostSystem() mo.HostSystem {
	hs := HostSystem
	hs.Config = NewHostConfigInfo()
	hostList = append(hostList, &hs)
	return hs
}

func GetHostList() []*mo.HostSystem {
	return hostList
}

func AddPnicToHost(h *mo.HostSystem, name, mac string) {
	key := "key-vim.host.PhysicalNic-" + name
	pnic := types.PhysicalNic{Key: key, Device: name, Mac: mac}
	h.Config.Network.Pnic = append(h.Config.Network.Pnic, pnic)
}

func DelPnicFromHost(h *mo.HostSystem, name string) {
	match := -1

	for ix, p := range h.Config.Network.Pnic {
		if p.Device == name {
			match = ix
			break
		}
	}

	if match != -1 {
		h.Config.Network.Pnic = append(h.Config.Network.Pnic[:match], h.Config.Network.Pnic[match+1:]...)
	}
}
