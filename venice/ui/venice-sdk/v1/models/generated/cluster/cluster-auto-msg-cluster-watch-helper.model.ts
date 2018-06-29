/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { ClusterAutoMsgClusterWatchHelperWatchEvent } from './cluster-auto-msg-cluster-watch-helper-watch-event.model';

export interface IClusterAutoMsgClusterWatchHelper {
    'Events'?: Array<ClusterAutoMsgClusterWatchHelperWatchEvent>;
}


export class ClusterAutoMsgClusterWatchHelper extends BaseModel implements IClusterAutoMsgClusterWatchHelper {
    'Events': Array<ClusterAutoMsgClusterWatchHelperWatchEvent>;
    public static enumProperties: { [key: string] : EnumDef } = {
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['Events'] = new Array<ClusterAutoMsgClusterWatchHelperWatchEvent>();
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
            this.fillModelArray<ClusterAutoMsgClusterWatchHelperWatchEvent>(this, 'Events', values['Events'], ClusterAutoMsgClusterWatchHelperWatchEvent);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'Events': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterAutoMsgClusterWatchHelperWatchEvent>('Events', this['Events'], ClusterAutoMsgClusterWatchHelperWatchEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this.fillModelArray<ClusterAutoMsgClusterWatchHelperWatchEvent>(this, 'Events', this['Events'], ClusterAutoMsgClusterWatchHelperWatchEvent);
        }
    }
}

