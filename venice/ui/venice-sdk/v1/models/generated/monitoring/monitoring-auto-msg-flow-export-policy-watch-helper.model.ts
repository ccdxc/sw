/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent, IMonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent } from './monitoring-auto-msg-flow-export-policy-watch-helper-watch-event.model';

export interface IMonitoringAutoMsgFlowExportPolicyWatchHelper {
    'Events'?: Array<IMonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent>;
}


export class MonitoringAutoMsgFlowExportPolicyWatchHelper extends BaseModel implements IMonitoringAutoMsgFlowExportPolicyWatchHelper {
    'Events': Array<MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'Events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringAutoMsgFlowExportPolicyWatchHelper.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringAutoMsgFlowExportPolicyWatchHelper.propInfo[prop] != null &&
                        MonitoringAutoMsgFlowExportPolicyWatchHelper.propInfo[prop].default != null &&
                        MonitoringAutoMsgFlowExportPolicyWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent>(this, 'Events', values['Events'], MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent>('Events', this['Events'], MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent>(this, 'Events', this['Events'], MonitoringAutoMsgFlowExportPolicyWatchHelperWatchEvent);
        }
    }
}

