/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SearchSearchRequest_mode,  } from './enums';
import { SearchSearchQuery, ISearchSearchQuery } from './search-search-query.model';

export interface ISearchSearchRequest {
    'query-string'?: string;
    'from': number;
    'max-results': number;
    'sort-by'?: string;
    'mode': SearchSearchRequest_mode;
    'query'?: ISearchSearchQuery;
    'tenants'?: Array<string>;
}


export class SearchSearchRequest extends BaseModel implements ISearchSearchRequest {
    /** length of string should be between 0 and 256
     */
    'query-string': string = null;
    /** value should be between 0 and 1023
     */
    'from': number = null;
    /** value should be between 0 and 8192
     */
    'max-results': number = null;
    /** SortyBy is an optional parameter and contains the field name 
    to be sorted by, For eg: "meta.name"
    This can be specified as URI parameter. */
    'sort-by': string = null;
    'mode': SearchSearchRequest_mode = null;
    /** Search query contains the search requirements
    This is intended for advanced query use cases involving
    boolean query, structured term query and supports various
    combinations of text, phrase strings and search modifiers
    for specific categories, kinds, fields and labels.
    This cannot be specified as URI parameter. */
    'query': SearchSearchQuery = null;
    'tenants': Array<string> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'query-string': {
            description:  'length of string should be between 0 and 256 ',
            type: 'string'
        },
        'from': {
            description:  'value should be between 0 and 1023 ',
            type: 'number'
        },
        'max-results': {
            default: '50',
            description:  'value should be between 0 and 8192 ',
            type: 'number'
        },
        'sort-by': {
            description:  'SortyBy is an optional parameter and contains the field name  to be sorted by, For eg: &quot;meta.name&quot; This can be specified as URI parameter.',
            type: 'string'
        },
        'mode': {
            enum: SearchSearchRequest_mode,
            default: 'Full',
            type: 'string'
        },
        'query': {
            description:  'Search query contains the search requirements This is intended for advanced query use cases involving boolean query, structured term query and supports various combinations of text, phrase strings and search modifiers for specific categories, kinds, fields and labels. This cannot be specified as URI parameter.',
            type: 'object'
        },
        'tenants': {
            type: 'Array<string>'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SearchSearchRequest.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SearchSearchRequest.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SearchSearchRequest.propInfo[prop] != null &&
                        SearchSearchRequest.propInfo[prop].default != null &&
                        SearchSearchRequest.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['query'] = new SearchSearchQuery();
        this['tenants'] = new Array<string>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['query-string'] != null) {
            this['query-string'] = values['query-string'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('query-string')) {
            this['query-string'] = SearchSearchRequest.propInfo['query-string'].default;
        } else {
            this['query-string'] = null
        }
        if (values && values['from'] != null) {
            this['from'] = values['from'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('from')) {
            this['from'] = SearchSearchRequest.propInfo['from'].default;
        } else {
            this['from'] = null
        }
        if (values && values['max-results'] != null) {
            this['max-results'] = values['max-results'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('max-results')) {
            this['max-results'] = SearchSearchRequest.propInfo['max-results'].default;
        } else {
            this['max-results'] = null
        }
        if (values && values['sort-by'] != null) {
            this['sort-by'] = values['sort-by'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('sort-by')) {
            this['sort-by'] = SearchSearchRequest.propInfo['sort-by'].default;
        } else {
            this['sort-by'] = null
        }
        if (values && values['mode'] != null) {
            this['mode'] = values['mode'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('mode')) {
            this['mode'] = <SearchSearchRequest_mode>  SearchSearchRequest.propInfo['mode'].default;
        } else {
            this['mode'] = null
        }
        if (values) {
            this['query'].setValues(values['query'], fillDefaults);
        } else {
            this['query'].setValues(null, fillDefaults);
        }
        if (values && values['tenants'] != null) {
            this['tenants'] = values['tenants'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('tenants')) {
            this['tenants'] = [ SearchSearchRequest.propInfo['tenants'].default];
        } else {
            this['tenants'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'query-string': CustomFormControl(new FormControl(this['query-string'], [maxLengthValidator(256), ]), SearchSearchRequest.propInfo['query-string'].description),
                'from': CustomFormControl(new FormControl(this['from'], [required, maxValueValidator(1023), ]), SearchSearchRequest.propInfo['from'].description),
                'max-results': CustomFormControl(new FormControl(this['max-results'], [required, maxValueValidator(8192), ]), SearchSearchRequest.propInfo['max-results'].description),
                'sort-by': CustomFormControl(new FormControl(this['sort-by'], [maxLengthValidator(256), ]), SearchSearchRequest.propInfo['sort-by'].description),
                'mode': CustomFormControl(new FormControl(this['mode'], [required, enumValidator(SearchSearchRequest_mode), ]), SearchSearchRequest.propInfo['mode'].description),
                'query': this['query'].$formGroup,
                'tenants': CustomFormControl(new FormControl(this['tenants']), SearchSearchRequest.propInfo['tenants'].description),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['query-string'].setValue(this['query-string']);
            this._formGroup.controls['from'].setValue(this['from']);
            this._formGroup.controls['max-results'].setValue(this['max-results']);
            this._formGroup.controls['sort-by'].setValue(this['sort-by']);
            this._formGroup.controls['mode'].setValue(this['mode']);
            this['query'].setFormGroupValuesToBeModelValues();
            this._formGroup.controls['tenants'].setValue(this['tenants']);
        }
    }
}

