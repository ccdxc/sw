/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';


export interface IClusterIPConfig {
    'ip-address'?: string;
    'default-gw'?: string;
    'dns-servers'?: Array<string>;
}


export class ClusterIPConfig extends BaseModel implements IClusterIPConfig {
    /** IPAddress contains the Management IP address of the DistributedServiceCard in CIDR format */
    'ip-address': string = null;
    /** DefaultGW contains the default gateway's IP address */
    'default-gw': string = null;
    /** DNSServers contains a list of DNS Servers that can be used on DistributedServiceCard */
    'dns-servers': Array<string> = null;
    public static propInfo: { [prop in keyof IClusterIPConfig]: PropInfoItem } = {
        'ip-address': {
            description:  'IPAddress contains the Management IP address of the DistributedServiceCard in CIDR format',
            required: false,
            type: 'string'
        },
        'default-gw': {
            description:  'DefaultGW contains the default gateway&#x27;s IP address',
            required: false,
            type: 'string'
        },
        'dns-servers': {
            description:  'DNSServers contains a list of DNS Servers that can be used on DistributedServiceCard',
            required: false,
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterIPConfig.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterIPConfig.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterIPConfig.propInfo[prop] != null &&
                        ClusterIPConfig.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['dns-servers'] = new Array<string>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['ip-address'] != null) {
            this['ip-address'] = values['ip-address'];
        } else if (fillDefaults && ClusterIPConfig.hasDefaultValue('ip-address')) {
            this['ip-address'] = ClusterIPConfig.propInfo['ip-address'].default;
        } else {
            this['ip-address'] = null
        }
        if (values && values['default-gw'] != null) {
            this['default-gw'] = values['default-gw'];
        } else if (fillDefaults && ClusterIPConfig.hasDefaultValue('default-gw')) {
            this['default-gw'] = ClusterIPConfig.propInfo['default-gw'].default;
        } else {
            this['default-gw'] = null
        }
        if (values && values['dns-servers'] != null) {
            this['dns-servers'] = values['dns-servers'];
        } else if (fillDefaults && ClusterIPConfig.hasDefaultValue('dns-servers')) {
            this['dns-servers'] = [ ClusterIPConfig.propInfo['dns-servers'].default];
        } else {
            this['dns-servers'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'ip-address': CustomFormControl(new FormControl(this['ip-address']), ClusterIPConfig.propInfo['ip-address']),
                'default-gw': CustomFormControl(new FormControl(this['default-gw']), ClusterIPConfig.propInfo['default-gw']),
                'dns-servers': CustomFormControl(new FormControl(this['dns-servers']), ClusterIPConfig.propInfo['dns-servers']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['ip-address'].setValue(this['ip-address']);
            this._formGroup.controls['default-gw'].setValue(this['default-gw']);
            this._formGroup.controls['dns-servers'].setValue(this['dns-servers']);
        }
    }
}

