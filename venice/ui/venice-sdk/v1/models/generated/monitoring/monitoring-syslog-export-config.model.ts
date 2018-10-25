/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IMonitoringSyslogExportConfig {
    'facility-override'?: string;
    'prefix'?: string;
}


export class MonitoringSyslogExportConfig extends BaseModel implements IMonitoringSyslogExportConfig {
    'facility-override': string = null;
    'prefix': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'facility-override': {
            type: 'string'
        },
        'prefix': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringSyslogExportConfig.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringSyslogExportConfig.propInfo[prop] != null &&
                        MonitoringSyslogExportConfig.propInfo[prop].default != null &&
                        MonitoringSyslogExportConfig.propInfo[prop].default != '');
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
        if (values && values['facility-override'] != null) {
            this['facility-override'] = values['facility-override'];
        } else if (fillDefaults && MonitoringSyslogExportConfig.hasDefaultValue('facility-override')) {
            this['facility-override'] = MonitoringSyslogExportConfig.propInfo['facility-override'].default;
        }
        if (values && values['prefix'] != null) {
            this['prefix'] = values['prefix'];
        } else if (fillDefaults && MonitoringSyslogExportConfig.hasDefaultValue('prefix')) {
            this['prefix'] = MonitoringSyslogExportConfig.propInfo['prefix'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'facility-override': new FormControl(this['facility-override']),
                'prefix': new FormControl(this['prefix']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['facility-override'].setValue(this['facility-override']);
            this._formGroup.controls['prefix'].setValue(this['prefix']);
        }
    }
}

