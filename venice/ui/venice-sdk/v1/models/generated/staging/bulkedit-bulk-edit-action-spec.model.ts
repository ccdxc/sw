/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { BulkeditBulkEditItem, IBulkeditBulkEditItem } from './bulkedit-bulk-edit-item.model';

export interface IBulkeditBulkEditActionSpec {
    'items'?: Array<IBulkeditBulkEditItem>;
    '_ui'?: any;
}


export class BulkeditBulkEditActionSpec extends BaseModel implements IBulkeditBulkEditActionSpec {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'items': Array<BulkeditBulkEditItem> = null;
    public static propInfo: { [prop in keyof IBulkeditBulkEditActionSpec]: PropInfoItem } = {
        'items': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return BulkeditBulkEditActionSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return BulkeditBulkEditActionSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (BulkeditBulkEditActionSpec.propInfo[prop] != null &&
                        BulkeditBulkEditActionSpec.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['items'] = new Array<BulkeditBulkEditItem>();
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
        if (values) {
            this.fillModelArray<BulkeditBulkEditItem>(this, 'items', values['items'], BulkeditBulkEditItem);
        } else {
            this['items'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<BulkeditBulkEditItem>('items', this['items'], BulkeditBulkEditItem);
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
            this.fillModelArray<BulkeditBulkEditItem>(this, 'items', this['items'], BulkeditBulkEditItem);
        }
    }
}

