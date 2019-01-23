/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringSyslogExport_format,  MonitoringSyslogExport_format_uihint  } from './enums';
import { MonitoringExportConfig, IMonitoringExportConfig } from './monitoring-export-config.model';
import { MonitoringSyslogExportConfig, IMonitoringSyslogExportConfig } from './monitoring-syslog-export-config.model';

export interface IMonitoringSyslogExport {
    'format': MonitoringSyslogExport_format;
    'targets'?: Array<IMonitoringExportConfig>;
    'config'?: IMonitoringSyslogExportConfig;
}


export class MonitoringSyslogExport extends BaseModel implements IMonitoringSyslogExport {
    'format': MonitoringSyslogExport_format = null;
    'targets': Array<MonitoringExportConfig> = null;
    'config': MonitoringSyslogExportConfig = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'format': {
            enum: MonitoringSyslogExport_format_uihint,
            default: 'SYSLOG_BSD',
            type: 'string'
        },
        'targets': {
            type: 'object'
        },
        'config': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringSyslogExport.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return MonitoringSyslogExport.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringSyslogExport.propInfo[prop] != null &&
                        MonitoringSyslogExport.propInfo[prop].default != null &&
                        MonitoringSyslogExport.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['targets'] = new Array<MonitoringExportConfig>();
        this['config'] = new MonitoringSyslogExportConfig();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['format'] != null) {
            this['format'] = values['format'];
        } else if (fillDefaults && MonitoringSyslogExport.hasDefaultValue('format')) {
            this['format'] = <MonitoringSyslogExport_format>  MonitoringSyslogExport.propInfo['format'].default;
        } else {
            this['format'] = null
        }
        if (values) {
            this.fillModelArray<MonitoringExportConfig>(this, 'targets', values['targets'], MonitoringExportConfig);
        } else {
            this['targets'] = [];
        }
        if (values) {
            this['config'].setValues(values['config'], fillDefaults);
        } else {
            this['config'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'format': new FormControl(this['format'], [required, enumValidator(MonitoringSyslogExport_format), ]),
                'targets': new FormArray([]),
                'config': this['config'].$formGroup,
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringExportConfig>('targets', this['targets'], MonitoringExportConfig);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['format'].setValue(this['format']);
            this.fillModelArray<MonitoringExportConfig>(this, 'targets', this['targets'], MonitoringExportConfig);
            this['config'].setFormGroupValuesToBeModelValues();
        }
    }
}

