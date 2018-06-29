/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { MonitoringAutoMsgAlertWatchHelperWatchEvent } from './monitoring-auto-msg-alert-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgAlertWatchHelper {
    'Events'?: Array<MonitoringAutoMsgAlertWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgAlertWatchHelper extends BaseModel implements IMonitoringAutoMsgAlertWatchHelper {
    'Events': Array<MonitoringAutoMsgAlertWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<MonitoringAutoMsgAlertWatchHelperWatchEvent>();
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
            this.fillModelArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>(this, 'Events', values['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>('Events', this['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgAlertWatchHelperWatchEvent>(this, 'Events', this['Events'], MonitoringAutoMsgAlertWatchHelperWatchEvent);
        }
    }
}

