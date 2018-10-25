/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterBiosInfo, IClusterBiosInfo } from './cluster-bios-info.model';
import { ClusterOsInfo, IClusterOsInfo } from './cluster-os-info.model';
import { ClusterCPUInfo, IClusterCPUInfo } from './cluster-cpu-info.model';
import { ClusterMemInfo, IClusterMemInfo } from './cluster-mem-info.model';
import { ClusterStorageInfo, IClusterStorageInfo } from './cluster-storage-info.model';

export interface IClusterSmartNICInfo {
    'bios-info'?: IClusterBiosInfo;
    'os-info'?: IClusterOsInfo;
    'cpu-info'?: IClusterCPUInfo;
    'memory-info'?: IClusterMemInfo;
    'storage-info'?: IClusterStorageInfo;
}


export class ClusterSmartNICInfo extends BaseModel implements IClusterSmartNICInfo {
    'bios-info': ClusterBiosInfo = null;
    'os-info': ClusterOsInfo = null;
    'cpu-info': ClusterCPUInfo = null;
    'memory-info': ClusterMemInfo = null;
    'storage-info': ClusterStorageInfo = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'bios-info': {
            type: 'object'
        },
        'os-info': {
            type: 'object'
        },
        'cpu-info': {
            type: 'object'
        },
        'memory-info': {
            type: 'object'
        },
        'storage-info': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterSmartNICInfo.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterSmartNICInfo.propInfo[prop] != null &&
                        ClusterSmartNICInfo.propInfo[prop].default != null &&
                        ClusterSmartNICInfo.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['bios-info'] = new ClusterBiosInfo();
        this['os-info'] = new ClusterOsInfo();
        this['cpu-info'] = new ClusterCPUInfo();
        this['memory-info'] = new ClusterMemInfo();
        this['storage-info'] = new ClusterStorageInfo();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this['bios-info'].setValues(values['bios-info']);
        }
        if (values) {
            this['os-info'].setValues(values['os-info']);
        }
        if (values) {
            this['cpu-info'].setValues(values['cpu-info']);
        }
        if (values) {
            this['memory-info'].setValues(values['memory-info']);
        }
        if (values) {
            this['storage-info'].setValues(values['storage-info']);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'bios-info': this['bios-info'].$formGroup,
                'os-info': this['os-info'].$formGroup,
                'cpu-info': this['cpu-info'].$formGroup,
                'memory-info': this['memory-info'].$formGroup,
                'storage-info': this['storage-info'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this['bios-info'].setFormGroupValuesToBeModelValues();
            this['os-info'].setFormGroupValuesToBeModelValues();
            this['cpu-info'].setFormGroupValuesToBeModelValues();
            this['memory-info'].setFormGroupValuesToBeModelValues();
            this['storage-info'].setFormGroupValuesToBeModelValues();
        }
    }
}

