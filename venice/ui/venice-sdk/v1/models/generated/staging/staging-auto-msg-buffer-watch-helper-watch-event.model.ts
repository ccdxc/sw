/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { StagingBuffer, IStagingBuffer } from './staging-buffer.model';

export interface IStagingAutoMsgBufferWatchHelperWatchEvent {
    'Type'?: string;
    'Object'?: IStagingBuffer;
}


export class StagingAutoMsgBufferWatchHelperWatchEvent extends BaseModel implements IStagingAutoMsgBufferWatchHelperWatchEvent {
    'Type': string = null;
    'Object': StagingBuffer = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'Type': {
            type: 'string'
        },
        'Object': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return StagingAutoMsgBufferWatchHelperWatchEvent.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (StagingAutoMsgBufferWatchHelperWatchEvent.propInfo[prop] != null &&
                        StagingAutoMsgBufferWatchHelperWatchEvent.propInfo[prop].default != null &&
                        StagingAutoMsgBufferWatchHelperWatchEvent.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Object'] = new StagingBuffer();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['Type'] != null) {
            this['Type'] = values['Type'];
        } else if (fillDefaults && StagingAutoMsgBufferWatchHelperWatchEvent.hasDefaultValue('Type')) {
            this['Type'] = StagingAutoMsgBufferWatchHelperWatchEvent.propInfo['Type'].default;
        }
        if (values) {
            this['Object'].setValues(values['Object']);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Type': new FormControl(this['Type']),
                'Object': this['Object'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['Type'].setValue(this['Type']);
            this['Object'].setFormGroupValuesToBeModelValues();
        }
    }
}

