/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { StagingCommitResponse_status,  } from './enums';

export interface IStagingCommitResponse {
    'status'?: StagingCommitResponse_status;
    'reason'?: string;
}


export class StagingCommitResponse extends BaseModel implements IStagingCommitResponse {
    'status': StagingCommitResponse_status = null;
    'reason': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'status': {
            enum: StagingCommitResponse_status,
            default: 'SUCCESS',
            type: 'string'
        },
        'reason': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return StagingCommitResponse.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (StagingCommitResponse.propInfo[prop] != null &&
                        StagingCommitResponse.propInfo[prop].default != null &&
                        StagingCommitResponse.propInfo[prop].default != '');
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
        if (values && values['status'] != null) {
            this['status'] = values['status'];
        } else if (fillDefaults && StagingCommitResponse.hasDefaultValue('status')) {
            this['status'] = <StagingCommitResponse_status>  StagingCommitResponse.propInfo['status'].default;
        }
        if (values && values['reason'] != null) {
            this['reason'] = values['reason'];
        } else if (fillDefaults && StagingCommitResponse.hasDefaultValue('reason')) {
            this['reason'] = StagingCommitResponse.propInfo['reason'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'status': new FormControl(this['status'], [enumValidator(StagingCommitResponse_status), ]),
                'reason': new FormControl(this['reason']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['status'].setValue(this['status']);
            this._formGroup.controls['reason'].setValue(this['reason']);
        }
    }
}

