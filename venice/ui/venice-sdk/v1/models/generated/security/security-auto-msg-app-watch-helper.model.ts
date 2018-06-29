/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { SecurityAutoMsgAppWatchHelperWatchEvent } from './security-auto-msg-app-watch-helper-watch-event.model';

export interface ISecurityAutoMsgAppWatchHelper {
    'Events'?: Array<SecurityAutoMsgAppWatchHelperWatchEvent>;
}


export class SecurityAutoMsgAppWatchHelper extends BaseModel implements ISecurityAutoMsgAppWatchHelper {
    'Events': Array<SecurityAutoMsgAppWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<SecurityAutoMsgAppWatchHelperWatchEvent>();
        if (values) {
            this.setValues(values);
        }
    }

    /**
     * set the values.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values) {
            this.fillModelArray<SecurityAutoMsgAppWatchHelperWatchEvent>(this, 'Events', values['Events'], SecurityAutoMsgAppWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<SecurityAutoMsgAppWatchHelperWatchEvent>('Events', this['Events'], SecurityAutoMsgAppWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<SecurityAutoMsgAppWatchHelperWatchEvent>(this, 'Events', this['Events'], SecurityAutoMsgAppWatchHelperWatchEvent);
        }
    }
}

