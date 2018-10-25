/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterPortCondition_type,  ClusterPortCondition_type_uihint  } from './enums';
import { ClusterPortCondition_status,  ClusterPortCondition_status_uihint  } from './enums';

export interface IClusterPortCondition {
    'type'?: ClusterPortCondition_type;
    'status'?: ClusterPortCondition_status;
    'last-transition-time'?: string;
    'reason'?: string;
    'message'?: string;
}


export class ClusterPortCondition extends BaseModel implements IClusterPortCondition {
    'type': ClusterPortCondition_type = null;
    'status': ClusterPortCondition_status = null;
    'last-transition-time': string = null;
    'reason': string = null;
    /** A detailed message indicating details about the transition. */
    'message': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'type': {
            enum: ClusterPortCondition_type_uihint,
            default: 'PORT_UP',
            type: 'string'
        },
        'status': {
            enum: ClusterPortCondition_status_uihint,
            default: 'UNKNOWN',
            type: 'string'
        },
        'last-transition-time': {
            type: 'string'
        },
        'reason': {
            type: 'string'
        },
        'message': {
            description:  'A detailed message indicating details about the transition.',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterPortCondition.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterPortCondition.propInfo[prop] != null &&
                        ClusterPortCondition.propInfo[prop].default != null &&
                        ClusterPortCondition.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (fillDefaults && ClusterPortCondition.hasDefaultValue('type')) {
            this['type'] = <ClusterPortCondition_type>  ClusterPortCondition.propInfo['type'].default;
        }
        if (values && values['status'] != null) {
            this['status'] = values['status'];
        } else if (fillDefaults && ClusterPortCondition.hasDefaultValue('status')) {
            this['status'] = <ClusterPortCondition_status>  ClusterPortCondition.propInfo['status'].default;
        }
        if (values && values['last-transition-time'] != null) {
            this['last-transition-time'] = values['last-transition-time'];
        } else if (fillDefaults && ClusterPortCondition.hasDefaultValue('last-transition-time')) {
            this['last-transition-time'] = ClusterPortCondition.propInfo['last-transition-time'].default;
        }
        if (values && values['reason'] != null) {
            this['reason'] = values['reason'];
        } else if (fillDefaults && ClusterPortCondition.hasDefaultValue('reason')) {
            this['reason'] = ClusterPortCondition.propInfo['reason'].default;
        }
        if (values && values['message'] != null) {
            this['message'] = values['message'];
        } else if (fillDefaults && ClusterPortCondition.hasDefaultValue('message')) {
            this['message'] = ClusterPortCondition.propInfo['message'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': new FormControl(this['type'], [enumValidator(ClusterPortCondition_type), ]),
                'status': new FormControl(this['status'], [enumValidator(ClusterPortCondition_status), ]),
                'last-transition-time': new FormControl(this['last-transition-time']),
                'reason': new FormControl(this['reason']),
                'message': new FormControl(this['message']),
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
            this._formGroup.controls['status'].setValue(this['status']);
            this._formGroup.controls['last-transition-time'].setValue(this['last-transition-time']);
            this._formGroup.controls['reason'].setValue(this['reason']);
            this._formGroup.controls['message'].setValue(this['message']);
        }
    }
}

