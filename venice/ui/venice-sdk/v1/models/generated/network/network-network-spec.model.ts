/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { NetworkNetworkSpec_type,  } from './enums';
import { NetworkRDSpec, INetworkRDSpec } from './network-rd-spec.model';
import { NetworkOrchestratorInfo, INetworkOrchestratorInfo } from './network-orchestrator-info.model';

export interface INetworkNetworkSpec {
    'type': NetworkNetworkSpec_type;
    'ipv4-subnet'?: string;
    'ipv4-gateway'?: string;
    'ipv6-subnet'?: string;
    'ipv6-gateway'?: string;
    'vlan-id': number;
    'vxlan-vni': number;
    'virtual-router'?: string;
    'ipam-policy'?: string;
    'route-import-export'?: INetworkRDSpec;
    'orchestrators'?: Array<INetworkOrchestratorInfo>;
    'ingress-security-policy'?: Array<string>;
    'egress-security-policy'?: Array<string>;
    '_ui'?: any;
}


export class NetworkNetworkSpec extends BaseModel implements INetworkNetworkSpec {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    /** Type of network. (vlan/vxlan/routed etc). */
    'type': NetworkNetworkSpec_type = null;
    /** IPv4 subnet CIDR. Should be a valid v4 or v6 CIDR block. */
    'ipv4-subnet': string = null;
    /** IPv4 gateway for this subnet. Should be a valid v4 or v6 IP address. */
    'ipv4-gateway': string = null;
    /** IPv6 subnet CIDR. */
    'ipv6-subnet': string = null;
    /** IPv6 gateway. */
    'ipv6-gateway': string = null;
    /** Vlan ID for the network. Value should be between 0 and 4095. */
    'vlan-id': number = null;
    /** Vxlan VNI for the network. Value should be between 0 and 16777215. */
    'vxlan-vni': number = null;
    /** VirtualRouter specifies the VRF this network belongs to. */
    'virtual-router': string = null;
    /** Relay Configuration if any. */
    'ipam-policy': string = null;
    /** RouteImportExport specifies what routes will be imported to this Router and how routes are tagged when exported. */
    'route-import-export': NetworkRDSpec = null;
    /** If supplied, this network will only be applied to the orchestrators specified. */
    'orchestrators': Array<NetworkOrchestratorInfo> = null;
    /** Security Policy to apply in the ingress direction. */
    'ingress-security-policy': Array<string> = null;
    /** Security Policy to apply in the egress direction. */
    'egress-security-policy': Array<string> = null;
    public static propInfo: { [prop in keyof INetworkNetworkSpec]: PropInfoItem } = {
        'type': {
            enum: NetworkNetworkSpec_type,
            default: 'bridged',
            description:  `Type of network. (vlan/vxlan/routed etc).`,
            required: true,
            type: 'string'
        },
        'ipv4-subnet': {
            description:  `IPv4 subnet CIDR. Should be a valid v4 or v6 CIDR block.`,
            hint:  '10.1.1.1/24, ff02::5/32 ',
            required: false,
            type: 'string'
        },
        'ipv4-gateway': {
            description:  `IPv4 gateway for this subnet. Should be a valid v4 or v6 IP address.`,
            hint:  '10.1.1.1, ff02::5 ',
            required: false,
            type: 'string'
        },
        'ipv6-subnet': {
            description:  `IPv6 subnet CIDR.`,
            required: false,
            type: 'string'
        },
        'ipv6-gateway': {
            description:  `IPv6 gateway.`,
            required: false,
            type: 'string'
        },
        'vlan-id': {
            description:  `Vlan ID for the network. Value should be between 0 and 4095.`,
            required: true,
            type: 'number'
        },
        'vxlan-vni': {
            description:  `Vxlan VNI for the network. Value should be between 0 and 16777215.`,
            required: true,
            type: 'number'
        },
        'virtual-router': {
            description:  `VirtualRouter specifies the VRF this network belongs to.`,
            required: false,
            type: 'string'
        },
        'ipam-policy': {
            description:  `Relay Configuration if any.`,
            required: false,
            type: 'string'
        },
        'route-import-export': {
            description:  `RouteImportExport specifies what routes will be imported to this Router and how routes are tagged when exported.`,
            required: false,
            type: 'object'
        },
        'orchestrators': {
            description:  `If supplied, this network will only be applied to the orchestrators specified.`,
            required: false,
            type: 'object'
        },
        'ingress-security-policy': {
            description:  `Security Policy to apply in the ingress direction.`,
            required: false,
            type: 'Array<string>'
        },
        'egress-security-policy': {
            description:  `Security Policy to apply in the egress direction.`,
            required: false,
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return NetworkNetworkSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return NetworkNetworkSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (NetworkNetworkSpec.propInfo[prop] != null &&
                        NetworkNetworkSpec.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['route-import-export'] = new NetworkRDSpec();
        this['orchestrators'] = new Array<NetworkOrchestratorInfo>();
        this['ingress-security-policy'] = new Array<string>();
        this['egress-security-policy'] = new Array<string>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('type')) {
            this['type'] = <NetworkNetworkSpec_type>  NetworkNetworkSpec.propInfo['type'].default;
        } else {
            this['type'] = null
        }
        if (values && values['ipv4-subnet'] != null) {
            this['ipv4-subnet'] = values['ipv4-subnet'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ipv4-subnet')) {
            this['ipv4-subnet'] = NetworkNetworkSpec.propInfo['ipv4-subnet'].default;
        } else {
            this['ipv4-subnet'] = null
        }
        if (values && values['ipv4-gateway'] != null) {
            this['ipv4-gateway'] = values['ipv4-gateway'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ipv4-gateway')) {
            this['ipv4-gateway'] = NetworkNetworkSpec.propInfo['ipv4-gateway'].default;
        } else {
            this['ipv4-gateway'] = null
        }
        if (values && values['ipv6-subnet'] != null) {
            this['ipv6-subnet'] = values['ipv6-subnet'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ipv6-subnet')) {
            this['ipv6-subnet'] = NetworkNetworkSpec.propInfo['ipv6-subnet'].default;
        } else {
            this['ipv6-subnet'] = null
        }
        if (values && values['ipv6-gateway'] != null) {
            this['ipv6-gateway'] = values['ipv6-gateway'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ipv6-gateway')) {
            this['ipv6-gateway'] = NetworkNetworkSpec.propInfo['ipv6-gateway'].default;
        } else {
            this['ipv6-gateway'] = null
        }
        if (values && values['vlan-id'] != null) {
            this['vlan-id'] = values['vlan-id'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('vlan-id')) {
            this['vlan-id'] = NetworkNetworkSpec.propInfo['vlan-id'].default;
        } else {
            this['vlan-id'] = null
        }
        if (values && values['vxlan-vni'] != null) {
            this['vxlan-vni'] = values['vxlan-vni'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('vxlan-vni')) {
            this['vxlan-vni'] = NetworkNetworkSpec.propInfo['vxlan-vni'].default;
        } else {
            this['vxlan-vni'] = null
        }
        if (values && values['virtual-router'] != null) {
            this['virtual-router'] = values['virtual-router'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('virtual-router')) {
            this['virtual-router'] = NetworkNetworkSpec.propInfo['virtual-router'].default;
        } else {
            this['virtual-router'] = null
        }
        if (values && values['ipam-policy'] != null) {
            this['ipam-policy'] = values['ipam-policy'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ipam-policy')) {
            this['ipam-policy'] = NetworkNetworkSpec.propInfo['ipam-policy'].default;
        } else {
            this['ipam-policy'] = null
        }
        if (values) {
            this['route-import-export'].setValues(values['route-import-export'], fillDefaults);
        } else {
            this['route-import-export'].setValues(null, fillDefaults);
        }
        if (values) {
            this.fillModelArray<NetworkOrchestratorInfo>(this, 'orchestrators', values['orchestrators'], NetworkOrchestratorInfo);
        } else {
            this['orchestrators'] = [];
        }
        if (values && values['ingress-security-policy'] != null) {
            this['ingress-security-policy'] = values['ingress-security-policy'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('ingress-security-policy')) {
            this['ingress-security-policy'] = [ NetworkNetworkSpec.propInfo['ingress-security-policy'].default];
        } else {
            this['ingress-security-policy'] = [];
        }
        if (values && values['egress-security-policy'] != null) {
            this['egress-security-policy'] = values['egress-security-policy'];
        } else if (fillDefaults && NetworkNetworkSpec.hasDefaultValue('egress-security-policy')) {
            this['egress-security-policy'] = [ NetworkNetworkSpec.propInfo['egress-security-policy'].default];
        } else {
            this['egress-security-policy'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': CustomFormControl(new FormControl(this['type'], [required, enumValidator(NetworkNetworkSpec_type), ]), NetworkNetworkSpec.propInfo['type']),
                'ipv4-subnet': CustomFormControl(new FormControl(this['ipv4-subnet']), NetworkNetworkSpec.propInfo['ipv4-subnet']),
                'ipv4-gateway': CustomFormControl(new FormControl(this['ipv4-gateway']), NetworkNetworkSpec.propInfo['ipv4-gateway']),
                'ipv6-subnet': CustomFormControl(new FormControl(this['ipv6-subnet']), NetworkNetworkSpec.propInfo['ipv6-subnet']),
                'ipv6-gateway': CustomFormControl(new FormControl(this['ipv6-gateway']), NetworkNetworkSpec.propInfo['ipv6-gateway']),
                'vlan-id': CustomFormControl(new FormControl(this['vlan-id'], [required, maxValueValidator(4095), ]), NetworkNetworkSpec.propInfo['vlan-id']),
                'vxlan-vni': CustomFormControl(new FormControl(this['vxlan-vni'], [required, maxValueValidator(16777215), ]), NetworkNetworkSpec.propInfo['vxlan-vni']),
                'virtual-router': CustomFormControl(new FormControl(this['virtual-router']), NetworkNetworkSpec.propInfo['virtual-router']),
                'ipam-policy': CustomFormControl(new FormControl(this['ipam-policy']), NetworkNetworkSpec.propInfo['ipam-policy']),
                'route-import-export': CustomFormGroup(this['route-import-export'].$formGroup, NetworkNetworkSpec.propInfo['route-import-export'].required),
                'orchestrators': new FormArray([]),
                'ingress-security-policy': CustomFormControl(new FormControl(this['ingress-security-policy']), NetworkNetworkSpec.propInfo['ingress-security-policy']),
                'egress-security-policy': CustomFormControl(new FormControl(this['egress-security-policy']), NetworkNetworkSpec.propInfo['egress-security-policy']),
            });
            // generate FormArray control elements
            this.fillFormArray<NetworkOrchestratorInfo>('orchestrators', this['orchestrators'], NetworkOrchestratorInfo);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('route-import-export') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('route-import-export').get(field);
                control.updateValueAndValidity();
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('orchestrators') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('orchestrators').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['type'].setValue(this['type']);
            this._formGroup.controls['ipv4-subnet'].setValue(this['ipv4-subnet']);
            this._formGroup.controls['ipv4-gateway'].setValue(this['ipv4-gateway']);
            this._formGroup.controls['ipv6-subnet'].setValue(this['ipv6-subnet']);
            this._formGroup.controls['ipv6-gateway'].setValue(this['ipv6-gateway']);
            this._formGroup.controls['vlan-id'].setValue(this['vlan-id']);
            this._formGroup.controls['vxlan-vni'].setValue(this['vxlan-vni']);
            this._formGroup.controls['virtual-router'].setValue(this['virtual-router']);
            this._formGroup.controls['ipam-policy'].setValue(this['ipam-policy']);
            this['route-import-export'].setFormGroupValuesToBeModelValues();
            this.fillModelArray<NetworkOrchestratorInfo>(this, 'orchestrators', this['orchestrators'], NetworkOrchestratorInfo);
            this._formGroup.controls['ingress-security-policy'].setValue(this['ingress-security-policy']);
            this._formGroup.controls['egress-security-policy'].setValue(this['egress-security-policy']);
        }
    }
}

