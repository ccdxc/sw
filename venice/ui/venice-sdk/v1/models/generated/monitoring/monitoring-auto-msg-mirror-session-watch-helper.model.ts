/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent } from './monitoring-auto-msg-mirror-session-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgMirrorSessionWatchHelper {
    'Events'?: Array<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgMirrorSessionWatchHelper extends BaseModel implements IMonitoringAutoMsgMirrorSessionWatchHelper {
    'Events': Array<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>();
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
            this.fillModelArray<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>(this, 'Events', values['Events'], MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>('Events', this['Events'], MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent>(this, 'Events', this['Events'], MonitoringAutoMsgMirrorSessionWatchHelperWatchEvent);
        }
    }
}

