/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterHostStatus_type,  } from './enums';

export interface IClusterHostStatus {
    'type'?: ClusterHostStatus_type;
    'admitted-smart-nics'?: Array<string>;
}


export class ClusterHostStatus extends BaseModel implements IClusterHostStatus {
    'type': ClusterHostStatus_type = null;
    'admitted-smart-nics': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'type': {
            enum: ClusterHostStatus_type,
            default: 'UNKNOWN',
            type: 'string'
        },
        'admitted-smart-nics': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterHostStatus.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterHostStatus.propInfo[prop] != null &&
                        ClusterHostStatus.propInfo[prop].default != null &&
                        ClusterHostStatus.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['admitted-smart-nics'] = new Array<string>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (fillDefaults && ClusterHostStatus.hasDefaultValue('type')) {
            this['type'] = <ClusterHostStatus_type>  ClusterHostStatus.propInfo['type'].default;
        }
        if (values && values['admitted-smart-nics'] != null) {
            this['admitted-smart-nics'] = values['admitted-smart-nics'];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': new FormControl(this['type'], [enumValidator(ClusterHostStatus_type), ]),
                'admitted-smart-nics': new FormControl(this['admitted-smart-nics']),
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
            this._formGroup.controls['admitted-smart-nics'].setValue(this['admitted-smart-nics']);
        }
    }
}

