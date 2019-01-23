/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent, IMonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent } from './monitoring-auto-msg-tech-support-request-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgTechSupportRequestWatchHelper {
    'events'?: Array<IMonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgTechSupportRequestWatchHelper extends BaseModel implements IMonitoringAutoMsgTechSupportRequestWatchHelper {
    'events': Array<MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgTechSupportRequestWatchHelper.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringAutoMsgTechSupportRequestWatchHelper.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgTechSupportRequestWatchHelper.propInfo[prop] != null &&
                        MonitoringAutoMsgTechSupportRequestWatchHelper.propInfo[prop].default != null &&
                        MonitoringAutoMsgTechSupportRequestWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['events'] = new Array<MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent>(this, 'events', values['events'], MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent);
        } else {
            this['events'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent>('events', this['events'], MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent>(this, 'events', this['events'], MonitoringAutoMsgTechSupportRequestWatchHelperWatchEvent);
        }
    }
}

