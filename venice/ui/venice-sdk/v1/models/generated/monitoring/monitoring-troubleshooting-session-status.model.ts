/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringTroubleshootingSessionStatus_state,  MonitoringTroubleshootingSessionStatus_state_uihint  } from './enums';
import { MonitoringTsResult, IMonitoringTsResult } from './monitoring-ts-result.model';

export interface IMonitoringTroubleshootingSessionStatus {
    'state'?: MonitoringTroubleshootingSessionStatus_state;
    'troubleshooting-results'?: Array<IMonitoringTsResult>;
}


export class MonitoringTroubleshootingSessionStatus extends BaseModel implements IMonitoringTroubleshootingSessionStatus {
    'state': MonitoringTroubleshootingSessionStatus_state = null;
    'troubleshooting-results': Array<MonitoringTsResult> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'state': {
            enum: MonitoringTroubleshootingSessionStatus_state_uihint,
            default: 'TS_RUNNING',
            type: 'string'
        },
        'troubleshooting-results': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringTroubleshootingSessionStatus.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringTroubleshootingSessionStatus.propInfo[prop] != null &&
                        MonitoringTroubleshootingSessionStatus.propInfo[prop].default != null &&
                        MonitoringTroubleshootingSessionStatus.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['troubleshooting-results'] = new Array<MonitoringTsResult>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['state'] != null) {
            this['state'] = values['state'];
        } else if (fillDefaults && MonitoringTroubleshootingSessionStatus.hasDefaultValue('state')) {
            this['state'] = <MonitoringTroubleshootingSessionStatus_state>  MonitoringTroubleshootingSessionStatus.propInfo['state'].default;
        }
        if (values) {
            this.fillModelArray<MonitoringTsResult>(this, 'troubleshooting-results', values['troubleshooting-results'], MonitoringTsResult);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'state': new FormControl(this['state'], [enumValidator(MonitoringTroubleshootingSessionStatus_state), ]),
                'troubleshooting-results': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringTsResult>('troubleshooting-results', this['troubleshooting-results'], MonitoringTsResult);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['state'].setValue(this['state']);
            this.fillModelArray<MonitoringTsResult>(this, 'troubleshooting-results', this['troubleshooting-results'], MonitoringTsResult);
        }
    }
}

