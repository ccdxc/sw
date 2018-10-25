/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringMatchSelector, IMonitoringMatchSelector } from './monitoring-match-selector.model';
import { MonitoringAppProtoSelector, IMonitoringAppProtoSelector } from './monitoring-app-proto-selector.model';

export interface IMonitoringMatchRule {
    'source'?: IMonitoringMatchSelector;
    'destination'?: IMonitoringMatchSelector;
    'app-protocol-selectors'?: IMonitoringAppProtoSelector;
}


export class MonitoringMatchRule extends BaseModel implements IMonitoringMatchRule {
    'source': MonitoringMatchSelector = null;
    'destination': MonitoringMatchSelector = null;
    'app-protocol-selectors': MonitoringAppProtoSelector = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'source': {
            type: 'object'
        },
        'destination': {
            type: 'object'
        },
        'app-protocol-selectors': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringMatchRule.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringMatchRule.propInfo[prop] != null &&
                        MonitoringMatchRule.propInfo[prop].default != null &&
                        MonitoringMatchRule.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['source'] = new MonitoringMatchSelector();
        this['destination'] = new MonitoringMatchSelector();
        this['app-protocol-selectors'] = new MonitoringAppProtoSelector();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this['source'].setValues(values['source']);
        }
        if (values) {
            this['destination'].setValues(values['destination']);
        }
        if (values) {
            this['app-protocol-selectors'].setValues(values['app-protocol-selectors']);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'source': this['source'].$formGroup,
                'destination': this['destination'].$formGroup,
                'app-protocol-selectors': this['app-protocol-selectors'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this['source'].setFormGroupValuesToBeModelValues();
            this['destination'].setFormGroupValuesToBeModelValues();
            this['app-protocol-selectors'].setFormGroupValuesToBeModelValues();
        }
    }
}

