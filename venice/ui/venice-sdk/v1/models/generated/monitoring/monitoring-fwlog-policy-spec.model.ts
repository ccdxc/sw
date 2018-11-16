/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { MonitoringFwlogPolicySpec_filter,  MonitoringFwlogPolicySpec_filter_uihint  } from './enums';
import { MonitoringFwlogExport, IMonitoringFwlogExport } from './monitoring-fwlog-export.model';

export interface IMonitoringFwlogPolicySpec {
    'retention-time'?: string;
    'filter'?: Array<MonitoringFwlogPolicySpec_filter>;
    'exports'?: Array<IMonitoringFwlogExport>;
}


export class MonitoringFwlogPolicySpec extends BaseModel implements IMonitoringFwlogPolicySpec {
    /** RetentionTime defines for how long to keep the fwlog before it is deleted. Default is 48h. */
    'retention-time': string = null;
    'filter': Array<MonitoringFwlogPolicySpec_filter> = null;
    'exports': Array<MonitoringFwlogExport> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'retention-time': {
            default: '48h',
            description:  'RetentionTime defines for how long to keep the fwlog before it is deleted. Default is 48h.',
            hint:  '2h',
            type: 'string'
        },
        'filter': {
            enum: MonitoringFwlogPolicySpec_filter_uihint,
            default: 'FWLOG_ALL',
            type: 'Array<string>'
        },
        'exports': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringFwlogPolicySpec.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringFwlogPolicySpec.propInfo[prop] != null &&
                        MonitoringFwlogPolicySpec.propInfo[prop].default != null &&
                        MonitoringFwlogPolicySpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['filter'] = new Array<MonitoringFwlogPolicySpec_filter>();
        this['exports'] = new Array<MonitoringFwlogExport>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['retention-time'] != null) {
            this['retention-time'] = values['retention-time'];
        } else if (fillDefaults && MonitoringFwlogPolicySpec.hasDefaultValue('retention-time')) {
            this['retention-time'] = MonitoringFwlogPolicySpec.propInfo['retention-time'].default;
        }
        if (values && values['filter'] != null) {
            this['filter'] = values['filter'];
        }
        if (values) {
            this.fillModelArray<MonitoringFwlogExport>(this, 'exports', values['exports'], MonitoringFwlogExport);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'retention-time': new FormControl(this['retention-time']),
                'filter': new FormControl(this['filter']),
                'exports': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<MonitoringFwlogExport>('exports', this['exports'], MonitoringFwlogExport);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['retention-time'].setValue(this['retention-time']);
            this._formGroup.controls['filter'].setValue(this['filter']);
            this.fillModelArray<MonitoringFwlogExport>(this, 'exports', this['exports'], MonitoringFwlogExport);
        }
    }
}

