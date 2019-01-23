/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { ClusterAutoMsgTenantWatchHelperWatchEvent, IClusterAutoMsgTenantWatchHelperWatchEvent } from './cluster-auto-msg-tenant-watch-helper-watch-event.model';

export interface IClusterAutoMsgTenantWatchHelper {
    'events'?: Array<IClusterAutoMsgTenantWatchHelperWatchEvent>;
}


export class ClusterAutoMsgTenantWatchHelper extends BaseModel implements IClusterAutoMsgTenantWatchHelper {
    'events': Array<ClusterAutoMsgTenantWatchHelperWatchEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'events': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterAutoMsgTenantWatchHelper.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterAutoMsgTenantWatchHelper.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterAutoMsgTenantWatchHelper.propInfo[prop] != null &&
                        ClusterAutoMsgTenantWatchHelper.propInfo[prop].default != null &&
                        ClusterAutoMsgTenantWatchHelper.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['events'] = new Array<ClusterAutoMsgTenantWatchHelperWatchEvent>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<ClusterAutoMsgTenantWatchHelperWatchEvent>(this, 'events', values['events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
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
            this.fillFormArray<ClusterAutoMsgTenantWatchHelperWatchEvent>('events', this['events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<ClusterAutoMsgTenantWatchHelperWatchEvent>(this, 'events', this['events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
        }
    }
}

