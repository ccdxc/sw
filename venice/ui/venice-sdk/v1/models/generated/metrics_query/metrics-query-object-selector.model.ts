/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { LabelsSelector, ILabelsSelector } from './labels-selector.model';

export interface IMetrics_queryObjectSelector {
    'name'?: string;
    'tenant'?: string;
    'namespace'?: string;
    'selector'?: ILabelsSelector;
}


export class Metrics_queryObjectSelector extends BaseModel implements IMetrics_queryObjectSelector {
    /** Name is the name of the API object. */
    'name': string = null;
    /** Tenant the object belongs to. */
    'tenant': string = null;
    /** Namespace the object belongs to. */
    'namespace': string = null;
    'selector': LabelsSelector = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'name': {
            description:  'Name is the name of the API object.',
            type: 'string'
        },
        'tenant': {
            description:  'Tenant the object belongs to.',
            type: 'string'
        },
        'namespace': {
            description:  'Namespace the object belongs to.',
            type: 'string'
        },
        'selector': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return Metrics_queryObjectSelector.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (Metrics_queryObjectSelector.propInfo[prop] != null &&
                        Metrics_queryObjectSelector.propInfo[prop].default != null &&
                        Metrics_queryObjectSelector.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['selector'] = new LabelsSelector();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['name'] != null) {
            this['name'] = values['name'];
        } else if (fillDefaults && Metrics_queryObjectSelector.hasDefaultValue('name')) {
            this['name'] = Metrics_queryObjectSelector.propInfo['name'].default;
        }
        if (values && values['tenant'] != null) {
            this['tenant'] = values['tenant'];
        } else if (fillDefaults && Metrics_queryObjectSelector.hasDefaultValue('tenant')) {
            this['tenant'] = Metrics_queryObjectSelector.propInfo['tenant'].default;
        }
        if (values && values['namespace'] != null) {
            this['namespace'] = values['namespace'];
        } else if (fillDefaults && Metrics_queryObjectSelector.hasDefaultValue('namespace')) {
            this['namespace'] = Metrics_queryObjectSelector.propInfo['namespace'].default;
        }
        if (values) {
            this['selector'].setValues(values['selector']);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'name': new FormControl(this['name']),
                'tenant': new FormControl(this['tenant']),
                'namespace': new FormControl(this['namespace']),
                'selector': this['selector'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['name'].setValue(this['name']);
            this._formGroup.controls['tenant'].setValue(this['tenant']);
            this._formGroup.controls['namespace'].setValue(this['namespace']);
            this['selector'].setFormGroupValuesToBeModelValues();
        }
    }
}

