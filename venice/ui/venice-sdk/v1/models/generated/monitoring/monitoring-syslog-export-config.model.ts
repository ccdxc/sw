/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringSyslogExportConfig_facility_override,  MonitoringSyslogExportConfig_facility_override_uihint  } from './enums';

export interface IMonitoringSyslogExportConfig {
    'facility-override': MonitoringSyslogExportConfig_facility_override;
    'prefix'?: string;
}


export class MonitoringSyslogExportConfig extends BaseModel implements IMonitoringSyslogExportConfig {
    'facility-override': MonitoringSyslogExportConfig_facility_override = null;
    'prefix': string = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'facility-override': {
            enum: MonitoringSyslogExportConfig_facility_override_uihint,
            default: 'LOG_USER',
            type: 'string'
        },
        'prefix': {
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringSyslogExportConfig.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringSyslogExportConfig.propInfo;
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
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['facility-override'] != null) {
            this['facility-override'] = values['facility-override'];
        } else if (fillDefaults && MonitoringSyslogExportConfig.hasDefaultValue('facility-override')) {
            this['facility-override'] = <MonitoringSyslogExportConfig_facility_override>  MonitoringSyslogExportConfig.propInfo['facility-override'].default;
        } else {
            this['facility-override'] = null
        }
        if (values && values['prefix'] != null) {
            this['prefix'] = values['prefix'];
        } else if (fillDefaults && MonitoringSyslogExportConfig.hasDefaultValue('prefix')) {
            this['prefix'] = MonitoringSyslogExportConfig.propInfo['prefix'].default;
        } else {
            this['prefix'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'facility-override': new FormControl(this['facility-override'], [required, enumValidator(MonitoringSyslogExportConfig_facility_override), ]),
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

