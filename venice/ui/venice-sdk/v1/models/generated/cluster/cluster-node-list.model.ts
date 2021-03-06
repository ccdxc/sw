/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { ApiListMeta, IApiListMeta } from './api-list-meta.model';
import { ClusterNode, IClusterNode } from './cluster-node.model';

export interface IClusterNodeList {
    'kind'?: string;
    'api-version'?: string;
    'list-meta'?: IApiListMeta;
    'items'?: Array<IClusterNode>;
    '_ui'?: any;
}


export class ClusterNodeList extends BaseModel implements IClusterNodeList {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'kind': string = null;
    'api-version': string = null;
    'list-meta': ApiListMeta = null;
    /** List of Node objects. */
    'items': Array<ClusterNode> = null;
    public static propInfo: { [prop in keyof IClusterNodeList]: PropInfoItem } = {
        'kind': {
            required: false,
            type: 'string'
        },
        'api-version': {
            required: false,
            type: 'string'
        },
        'list-meta': {
            required: false,
            type: 'object'
        },
        'items': {
            description:  `List of Node objects.`,
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterNodeList.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterNodeList.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterNodeList.propInfo[prop] != null &&
                        ClusterNodeList.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['list-meta'] = new ApiListMeta();
        this['items'] = new Array<ClusterNode>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (fillDefaults && ClusterNodeList.hasDefaultValue('kind')) {
            this['kind'] = ClusterNodeList.propInfo['kind'].default;
        } else {
            this['kind'] = null
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (fillDefaults && ClusterNodeList.hasDefaultValue('api-version')) {
            this['api-version'] = ClusterNodeList.propInfo['api-version'].default;
        } else {
            this['api-version'] = null
        }
        if (values) {
            this['list-meta'].setValues(values['list-meta'], fillDefaults);
        } else {
            this['list-meta'].setValues(null, fillDefaults);
        }
        if (values) {
            this.fillModelArray<ClusterNode>(this, 'items', values['items'], ClusterNode);
        } else {
            this['items'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': CustomFormControl(new FormControl(this['kind']), ClusterNodeList.propInfo['kind']),
                'api-version': CustomFormControl(new FormControl(this['api-version']), ClusterNodeList.propInfo['api-version']),
                'list-meta': CustomFormGroup(this['list-meta'].$formGroup, ClusterNodeList.propInfo['list-meta'].required),
                'items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterNode>('items', this['items'], ClusterNode);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('list-meta') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('list-meta').get(field);
                control.updateValueAndValidity();
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('items') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('items').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this['list-meta'].setFormGroupValuesToBeModelValues();
            this.fillModelArray<ClusterNode>(this, 'items', this['items'], ClusterNode);
        }
    }
}

