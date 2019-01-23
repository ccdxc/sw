/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { WorkloadWorkloadIntfSpec, IWorkloadWorkloadIntfSpec } from './workload-workload-intf-spec.model';

export interface IWorkloadWorkloadSpec {
    'host-name': string;
    'interfaces'?: Array<IWorkloadWorkloadIntfSpec>;
}


export class WorkloadWorkloadSpec extends BaseModel implements IWorkloadWorkloadSpec {
    /** should be a valid host address, IP address or hostname
     */
    'host-name': string = null;
    'interfaces': Array<WorkloadWorkloadIntfSpec> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'host-name': {
            description:  'should be a valid host address, IP address or hostname ',
            hint:  '10.1.1.1, ff02::5, localhost, example.domain.com ',
            type: 'string'
        },
        'interfaces': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return WorkloadWorkloadSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return WorkloadWorkloadSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (WorkloadWorkloadSpec.propInfo[prop] != null &&
                        WorkloadWorkloadSpec.propInfo[prop].default != null &&
                        WorkloadWorkloadSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['interfaces'] = new Array<WorkloadWorkloadIntfSpec>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['host-name'] != null) {
            this['host-name'] = values['host-name'];
        } else if (fillDefaults && WorkloadWorkloadSpec.hasDefaultValue('host-name')) {
            this['host-name'] = WorkloadWorkloadSpec.propInfo['host-name'].default;
        } else {
            this['host-name'] = null
        }
        if (values) {
            this.fillModelArray<WorkloadWorkloadIntfSpec>(this, 'interfaces', values['interfaces'], WorkloadWorkloadIntfSpec);
        } else {
            this['interfaces'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'host-name': new FormControl(this['host-name'], [required, ]),
                'interfaces': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<WorkloadWorkloadIntfSpec>('interfaces', this['interfaces'], WorkloadWorkloadIntfSpec);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['host-name'].setValue(this['host-name']);
            this.fillModelArray<WorkloadWorkloadIntfSpec>(this, 'interfaces', this['interfaces'], WorkloadWorkloadIntfSpec);
        }
    }
}

