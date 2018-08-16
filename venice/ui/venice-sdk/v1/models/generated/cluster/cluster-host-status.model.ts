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
    'operating-system'?: string;
    'orchestrator'?: string;
    'interfaces'?: object;
}


export class ClusterHostStatus extends BaseModel implements IClusterHostStatus {
    'type': ClusterHostStatus_type = null;
    'operating-system': string = null;
    /** Orchestrator is the name of associated Compute controller
    (like VCenter) managing this host. */
    'orchestrator': string = null;
    'interfaces': object = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'type': {
            enum: ClusterHostStatus_type,
            default: 'UNKNOWN',
            type: 'string'
        },
        'operating-system': {
            type: 'string'
        },
        'orchestrator': {
            description:  'Orchestrator is the name of associated Compute controller (like VCenter) managing this host.',
            type: 'string'
        },
        'interfaces': {
            type: 'object'
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
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['type'] != null) {
            this['type'] = values['type'];
        } else if (ClusterHostStatus.hasDefaultValue('type')) {
            this['type'] = <ClusterHostStatus_type>  ClusterHostStatus.propInfo['type'].default;
        }
        if (values && values['operating-system'] != null) {
            this['operating-system'] = values['operating-system'];
        } else if (ClusterHostStatus.hasDefaultValue('operating-system')) {
            this['operating-system'] = ClusterHostStatus.propInfo['operating-system'].default;
        }
        if (values && values['orchestrator'] != null) {
            this['orchestrator'] = values['orchestrator'];
        } else if (ClusterHostStatus.hasDefaultValue('orchestrator')) {
            this['orchestrator'] = ClusterHostStatus.propInfo['orchestrator'].default;
        }
        if (values && values['interfaces'] != null) {
            this['interfaces'] = values['interfaces'];
        } else if (ClusterHostStatus.hasDefaultValue('interfaces')) {
            this['interfaces'] = ClusterHostStatus.propInfo['interfaces'].default;
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'type': new FormControl(this['type'], [enumValidator(ClusterHostStatus_type), ]),
                'operating-system': new FormControl(this['operating-system']),
                'orchestrator': new FormControl(this['orchestrator']),
                'interfaces': new FormControl(this['interfaces']),
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['type'].setValue(this['type']);
            this._formGroup.controls['operating-system'].setValue(this['operating-system']);
            this._formGroup.controls['orchestrator'].setValue(this['orchestrator']);
            this._formGroup.controls['interfaces'].setValue(this['interfaces']);
        }
    }
}

