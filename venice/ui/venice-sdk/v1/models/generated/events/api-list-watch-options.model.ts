/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';


export interface IApiListWatchOptions {
    'name'?: string;
    'tenant'?: string;
    'namespace'?: string;
    'generation-id'?: string;
    'resource-version'?: string;
    'uuid'?: string;
    'labels'?: object;
    'creation-time'?: Date;
    'mod-time'?: Date;
    'self-link'?: string;
    'label-selector'?: string;
    'field-selector'?: string;
    'field-change-selector'?: Array<string>;
    'from'?: number;
    'max-results'?: number;
}


export class ApiListWatchOptions extends BaseModel implements IApiListWatchOptions {
    'name': string = null;
    'tenant': string = null;
    'namespace': string = null;
    'generation-id': string = null;
    'resource-version': string = null;
    'uuid': string = null;
    'labels': object = null;
    'creation-time': Date = null;
    'mod-time': Date = null;
    'self-link': string = null;
    /** LabelSelector to select on labels in list or watch results. */
    'label-selector': string = null;
    'field-selector': string = null;
    /** FieldChangeSelector specifies to generate a watch notification on change in field(s) specified. */
    'field-change-selector': Array<string> = null;
    'from': number = null;
    /** max. number of events to be fetched for the request. */
    'max-results': number = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'name': {
            type: 'string'
        },
        'tenant': {
            type: 'string'
        },
        'namespace': {
            type: 'string'
        },
        'generation-id': {
            type: 'string'
        },
        'resource-version': {
            type: 'string'
        },
        'uuid': {
            type: 'string'
        },
        'labels': {
            type: 'object'
        },
        'creation-time': {
            type: 'Date'
        },
        'mod-time': {
            type: 'Date'
        },
        'self-link': {
            type: 'string'
        },
        'label-selector': {
            description:  'LabelSelector to select on labels in list or watch results.',
            type: 'string'
        },
        'field-selector': {
            type: 'string'
        },
        'field-change-selector': {
            description:  'FieldChangeSelector specifies to generate a watch notification on change in field(s) specified.',
            type: 'Array<string>'
        },
        'from': {
            type: 'number'
        },
        'max-results': {
            description:  'max. number of events to be fetched for the request.',
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ApiListWatchOptions.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ApiListWatchOptions.propInfo[prop] != null &&
                        ApiListWatchOptions.propInfo[prop].default != null &&
                        ApiListWatchOptions.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['field-change-selector'] = new Array<string>();
        this.setValues(values);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['name'] != null) {
            this['name'] = values['name'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('name')) {
            this['name'] = ApiListWatchOptions.propInfo['name'].default;
        }
        if (values && values['tenant'] != null) {
            this['tenant'] = values['tenant'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('tenant')) {
            this['tenant'] = ApiListWatchOptions.propInfo['tenant'].default;
        }
        if (values && values['namespace'] != null) {
            this['namespace'] = values['namespace'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('namespace')) {
            this['namespace'] = ApiListWatchOptions.propInfo['namespace'].default;
        }
        if (values && values['generation-id'] != null) {
            this['generation-id'] = values['generation-id'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('generation-id')) {
            this['generation-id'] = ApiListWatchOptions.propInfo['generation-id'].default;
        }
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('resource-version')) {
            this['resource-version'] = ApiListWatchOptions.propInfo['resource-version'].default;
        }
        if (values && values['uuid'] != null) {
            this['uuid'] = values['uuid'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('uuid')) {
            this['uuid'] = ApiListWatchOptions.propInfo['uuid'].default;
        }
        if (values && values['labels'] != null) {
            this['labels'] = values['labels'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('labels')) {
            this['labels'] = ApiListWatchOptions.propInfo['labels'].default;
        }
        if (values && values['creation-time'] != null) {
            this['creation-time'] = values['creation-time'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('creation-time')) {
            this['creation-time'] = ApiListWatchOptions.propInfo['creation-time'].default;
        }
        if (values && values['mod-time'] != null) {
            this['mod-time'] = values['mod-time'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('mod-time')) {
            this['mod-time'] = ApiListWatchOptions.propInfo['mod-time'].default;
        }
        if (values && values['self-link'] != null) {
            this['self-link'] = values['self-link'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('self-link')) {
            this['self-link'] = ApiListWatchOptions.propInfo['self-link'].default;
        }
        if (values && values['label-selector'] != null) {
            this['label-selector'] = values['label-selector'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('label-selector')) {
            this['label-selector'] = ApiListWatchOptions.propInfo['label-selector'].default;
        }
        if (values && values['field-selector'] != null) {
            this['field-selector'] = values['field-selector'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('field-selector')) {
            this['field-selector'] = ApiListWatchOptions.propInfo['field-selector'].default;
        }
        if (values && values['field-change-selector'] != null) {
            this['field-change-selector'] = values['field-change-selector'];
        }
        if (values && values['from'] != null) {
            this['from'] = values['from'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('from')) {
            this['from'] = ApiListWatchOptions.propInfo['from'].default;
        }
        if (values && values['max-results'] != null) {
            this['max-results'] = values['max-results'];
        } else if (fillDefaults && ApiListWatchOptions.hasDefaultValue('max-results')) {
            this['max-results'] = ApiListWatchOptions.propInfo['max-results'].default;
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'name': new FormControl(this['name']),
                'tenant': new FormControl(this['tenant']),
                'namespace': new FormControl(this['namespace']),
                'generation-id': new FormControl(this['generation-id']),
                'resource-version': new FormControl(this['resource-version']),
                'uuid': new FormControl(this['uuid']),
                'labels': new FormControl(this['labels']),
                'creation-time': new FormControl(this['creation-time']),
                'mod-time': new FormControl(this['mod-time']),
                'self-link': new FormControl(this['self-link']),
                'label-selector': new FormControl(this['label-selector']),
                'field-selector': new FormControl(this['field-selector']),
                'field-change-selector': new FormControl(this['field-change-selector']),
                'from': new FormControl(this['from']),
                'max-results': new FormControl(this['max-results']),
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
            this._formGroup.controls['generation-id'].setValue(this['generation-id']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this._formGroup.controls['uuid'].setValue(this['uuid']);
            this._formGroup.controls['labels'].setValue(this['labels']);
            this._formGroup.controls['creation-time'].setValue(this['creation-time']);
            this._formGroup.controls['mod-time'].setValue(this['mod-time']);
            this._formGroup.controls['self-link'].setValue(this['self-link']);
            this._formGroup.controls['label-selector'].setValue(this['label-selector']);
            this._formGroup.controls['field-selector'].setValue(this['field-selector']);
            this._formGroup.controls['field-change-selector'].setValue(this['field-change-selector']);
            this._formGroup.controls['from'].setValue(this['from']);
            this._formGroup.controls['max-results'].setValue(this['max-results']);
        }
    }
}

