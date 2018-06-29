/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { ClusterAutoMsgTenantWatchHelperWatchEvent } from './cluster-auto-msg-tenant-watch-helper-watch-event.model';

export interface IClusterAutoMsgTenantWatchHelper {
    'Events'?: Array<ClusterAutoMsgTenantWatchHelperWatchEvent>;
}


export class ClusterAutoMsgTenantWatchHelper extends BaseModel implements IClusterAutoMsgTenantWatchHelper {
    'Events': Array<ClusterAutoMsgTenantWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<ClusterAutoMsgTenantWatchHelperWatchEvent>();
        if (values) {
            this.setValues(values);
        }
    }

    /**
     * set the values.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values) {
            this.fillModelArray<ClusterAutoMsgTenantWatchHelperWatchEvent>(this, 'Events', values['Events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterAutoMsgTenantWatchHelperWatchEvent>('Events', this['Events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<ClusterAutoMsgTenantWatchHelperWatchEvent>(this, 'Events', this['Events'], ClusterAutoMsgTenantWatchHelperWatchEvent);
        }
    }
}

