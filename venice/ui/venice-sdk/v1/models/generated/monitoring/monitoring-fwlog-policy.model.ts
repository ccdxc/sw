/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ApiObjectMeta, IApiObjectMeta } from './api-object-meta.model';
import { MonitoringFwlogSpec, IMonitoringFwlogSpec } from './monitoring-fwlog-spec.model';
import { MonitoringFwlogStatus, IMonitoringFwlogStatus } from './monitoring-fwlog-status.model';

export interface IMonitoringFwlogPolicy {
    'kind'?: string;
    'api-version'?: string;
    'meta'?: IApiObjectMeta;
    'spec'?: IMonitoringFwlogSpec;
    'status'?: IMonitoringFwlogStatus;
}


export class MonitoringFwlogPolicy extends BaseModel implements IMonitoringFwlogPolicy {
    'kind': string = null;
    'api-version': string = null;
    'meta': ApiObjectMeta = null;
    'spec': MonitoringFwlogSpec = null;
    /** Status contains the current state of the policy. */
    'status': MonitoringFwlogStatus = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'meta': {
            type: 'object'
        },
        'spec': {
            type: 'object'
        },
        'status': {
            description:  'Status contains the current state of the policy.',
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return MonitoringFwlogPolicy.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (MonitoringFwlogPolicy.propInfo[prop] != null &&
                        MonitoringFwlogPolicy.propInfo[prop].default != null &&
                        MonitoringFwlogPolicy.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['meta'] = new ApiObjectMeta();
        this['spec'] = new MonitoringFwlogSpec();
        this['status'] = new MonitoringFwlogStatus();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (MonitoringFwlogPolicy.hasDefaultValue('kind')) {
            this['kind'] = MonitoringFwlogPolicy.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (MonitoringFwlogPolicy.hasDefaultValue('api-version')) {
            this['api-version'] = MonitoringFwlogPolicy.propInfo['api-version'].default;
        }
        if (values) {
            this['meta'].setValues(values['meta']);
        }
        if (values) {
            this['spec'].setValues(values['spec']);
        }
        if (values) {
            this['status'].setValues(values['status']);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'meta': this['meta'].$formGroup,
                'spec': this['spec'].$formGroup,
                'status': this['status'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this['meta'].setFormGroupValues();
            this['spec'].setFormGroupValues();
            this['status'].setFormGroupValues();
        }
    }
}

