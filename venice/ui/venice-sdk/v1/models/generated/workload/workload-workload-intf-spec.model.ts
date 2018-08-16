/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IWorkloadWorkloadIntfSpec {
    'micro-seg-vlan'?: number;
    'external-vlan'?: number;
}


export class WorkloadWorkloadIntfSpec extends BaseModel implements IWorkloadWorkloadIntfSpec {
    /** value should be between 1 and 4095
     */
    'micro-seg-vlan': number = null;
    /** value should be between 1 and 4095
     */
    'external-vlan': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'micro-seg-vlan': {
            description:  'value should be between 1 and 4095 ',
            type: 'number'
        },
        'external-vlan': {
            description:  'value should be between 1 and 4095 ',
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return WorkloadWorkloadIntfSpec.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (WorkloadWorkloadIntfSpec.propInfo[prop] != null &&
                        WorkloadWorkloadIntfSpec.propInfo[prop].default != null &&
                        WorkloadWorkloadIntfSpec.propInfo[prop].default != '');
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
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['micro-seg-vlan'] != null) {
            this['micro-seg-vlan'] = values['micro-seg-vlan'];
        } else if (WorkloadWorkloadIntfSpec.hasDefaultValue('micro-seg-vlan')) {
            this['micro-seg-vlan'] = WorkloadWorkloadIntfSpec.propInfo['micro-seg-vlan'].default;
        }
        if (values && values['external-vlan'] != null) {
            this['external-vlan'] = values['external-vlan'];
        } else if (WorkloadWorkloadIntfSpec.hasDefaultValue('external-vlan')) {
            this['external-vlan'] = WorkloadWorkloadIntfSpec.propInfo['external-vlan'].default;
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'micro-seg-vlan': new FormControl(this['micro-seg-vlan'], [minValueValidator(1), maxValueValidator(4095), ]),
                'external-vlan': new FormControl(this['external-vlan'], [minValueValidator(1), maxValueValidator(4095), ]),
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['micro-seg-vlan'].setValue(this['micro-seg-vlan']);
            this._formGroup.controls['external-vlan'].setValue(this['external-vlan']);
        }
    }
}

