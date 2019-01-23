/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SearchTextRequirement, ISearchTextRequirement } from './search-text-requirement.model';
import { SearchSearchQuery_categories,  } from './enums';
import { SearchSearchQuery_kinds,  } from './enums';
import { FieldsSelector, IFieldsSelector } from './fields-selector.model';
import { LabelsSelector, ILabelsSelector } from './labels-selector.model';

export interface ISearchSearchQuery {
    'texts'?: Array<ISearchTextRequirement>;
    'categories'?: Array<SearchSearchQuery_categories>;
    'kinds'?: Array<SearchSearchQuery_kinds>;
    'fields'?: IFieldsSelector;
    'labels'?: ILabelsSelector;
}


export class SearchSearchQuery extends BaseModel implements ISearchSearchQuery {
    'texts': Array<SearchTextRequirement> = null;
    /** length of string should be between 0 and 64
     */
    'categories': Array<SearchSearchQuery_categories> = null;
    /** length of string should be between 0 and 64
     */
    'kinds': Array<SearchSearchQuery_kinds> = null;
    'fields': FieldsSelector = null;
    'labels': LabelsSelector = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'texts': {
            type: 'object'
        },
        'categories': {
            enum: SearchSearchQuery_categories,
            default: 'Cluster',
            description:  'length of string should be between 0 and 64 ',
            type: 'Array<string>'
        },
        'kinds': {
            enum: SearchSearchQuery_kinds,
            default: 'Cluster',
            description:  'length of string should be between 0 and 64 ',
            type: 'Array<string>'
        },
        'fields': {
            type: 'object'
        },
        'labels': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SearchSearchQuery.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SearchSearchQuery.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SearchSearchQuery.propInfo[prop] != null &&
                        SearchSearchQuery.propInfo[prop].default != null &&
                        SearchSearchQuery.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['texts'] = new Array<SearchTextRequirement>();
        this['categories'] = new Array<SearchSearchQuery_categories>();
        this['kinds'] = new Array<SearchSearchQuery_kinds>();
        this['fields'] = new FieldsSelector();
        this['labels'] = new LabelsSelector();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this.fillModelArray<SearchTextRequirement>(this, 'texts', values['texts'], SearchTextRequirement);
        } else {
            this['texts'] = [];
        }
        if (values && values['categories'] != null) {
            this['categories'] = values['categories'];
        } else if (fillDefaults && SearchSearchQuery.hasDefaultValue('categories')) {
            this['categories'] = [ SearchSearchQuery.propInfo['categories'].default];
        } else {
            this['categories'] = [];
        }
        if (values && values['kinds'] != null) {
            this['kinds'] = values['kinds'];
        } else if (fillDefaults && SearchSearchQuery.hasDefaultValue('kinds')) {
            this['kinds'] = [ SearchSearchQuery.propInfo['kinds'].default];
        } else {
            this['kinds'] = [];
        }
        if (values) {
            this['fields'].setValues(values['fields'], fillDefaults);
        } else {
            this['fields'].setValues(null, fillDefaults);
        }
        if (values) {
            this['labels'].setValues(values['labels'], fillDefaults);
        } else {
            this['labels'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'texts': new FormArray([]),
                'categories': new FormControl(this['categories']),
                'kinds': new FormControl(this['kinds']),
                'fields': this['fields'].$formGroup,
                'labels': this['labels'].$formGroup,
            });
            // generate FormArray control elements
            this.fillFormArray<SearchTextRequirement>('texts', this['texts'], SearchTextRequirement);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this.fillModelArray<SearchTextRequirement>(this, 'texts', this['texts'], SearchTextRequirement);
            this._formGroup.controls['categories'].setValue(this['categories']);
            this._formGroup.controls['kinds'].setValue(this['kinds']);
            this['fields'].setFormGroupValuesToBeModelValues();
            this['labels'].setFormGroupValuesToBeModelValues();
        }
    }
}

