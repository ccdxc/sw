/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, EnumDef } from './base-model';

import { ClusterSmartNICSpec_phase,  ClusterSmartNICSpec_phase_uihint  } from './enums';
import { ClusterPortSpec } from './cluster-port-spec.model';

export interface IClusterSmartNICSpec {
    'phase'?: ClusterSmartNICSpec_phase;
    'mgmt-ip'?: string;
    'host-name'?: string;
    'ports'?: Array<ClusterPortSpec>;
}


export class ClusterSmartNICSpec extends BaseModel implements IClusterSmartNICSpec {
    /** Current phase of the SmartNIC.
When auto-admission is enabled, Phase will be set to NIC_ADMITTED
by CMD for validated NICs.
When auto-admission is not enabled, Phase will be set to NIC_PENDING
by CMD for validated NICs since it requires manual approval.
To admit the NIC as a part of manual admission, user is expected to
set the Phase to NIC_ADMITTED for the NICs that are in NIC_PENDING
state. Note : Whitelist mode is not supported yet. */
    'phase': ClusterSmartNICSpec_phase;
    'mgmt-ip': string;
    'host-name': string;
    'ports': Array<ClusterPortSpec>;
    public static enumProperties: { [key: string] : EnumDef } = {
        'phase': {
            enum: ClusterSmartNICSpec_phase_uihint,
            default: 'UNKNOWN',
        },
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['ports'] = new Array<ClusterPortSpec>();
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
            this['phase'] = values['phase'];
            this['mgmt-ip'] = values['mgmt-ip'];
            this['host-name'] = values['host-name'];
            this.fillModelArray<ClusterPortSpec>(this, 'ports', values['ports'], ClusterPortSpec);
        }
    }

    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'phase': new FormControl(this['phase'], [enumValidator(ClusterSmartNICSpec_phase), ]),
                'mgmt-ip': new FormControl(this['mgmt-ip']),
                'host-name': new FormControl(this['host-name']),
                'ports': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterPortSpec>('ports', this['ports'], ClusterPortSpec);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['phase'].setValue(this['phase']);
            this._formGroup.controls['mgmt-ip'].setValue(this['mgmt-ip']);
            this._formGroup.controls['host-name'].setValue(this['host-name']);
            this.fillModelArray<ClusterPortSpec>(this, 'ports', this['ports'], ClusterPortSpec);
        }
    }
}

