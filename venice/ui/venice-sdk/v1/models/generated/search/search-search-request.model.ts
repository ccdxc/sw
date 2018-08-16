/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { SearchSearchRequest_mode,  } from './enums';
import { SearchSearchQuery, ISearchSearchQuery } from './search-search-query.model';

export interface ISearchSearchRequest {
    'query-string'?: string;
    'from'?: number;
    'max-results'?: number;
    'sort-by'?: string;
    'mode'?: SearchSearchRequest_mode;
    'query'?: ISearchSearchQuery;
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
            default: '10',
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
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SearchSearchRequest.propInfo[propName];
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
    constructor(values?: any) {
        super();
        this['query'] = new SearchSearchQuery();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['query-string'] != null) {
            this['query-string'] = values['query-string'];
        } else if (SearchSearchRequest.hasDefaultValue('query-string')) {
            this['query-string'] = SearchSearchRequest.propInfo['query-string'].default;
        }
        if (values && values['from'] != null) {
            this['from'] = values['from'];
        } else if (SearchSearchRequest.hasDefaultValue('from')) {
            this['from'] = SearchSearchRequest.propInfo['from'].default;
        }
        if (values && values['max-results'] != null) {
            this['max-results'] = values['max-results'];
        } else if (SearchSearchRequest.hasDefaultValue('max-results')) {
            this['max-results'] = SearchSearchRequest.propInfo['max-results'].default;
        }
        if (values && values['sort-by'] != null) {
            this['sort-by'] = values['sort-by'];
        } else if (SearchSearchRequest.hasDefaultValue('sort-by')) {
            this['sort-by'] = SearchSearchRequest.propInfo['sort-by'].default;
        }
        if (values && values['mode'] != null) {
            this['mode'] = values['mode'];
        } else if (SearchSearchRequest.hasDefaultValue('mode')) {
            this['mode'] = <SearchSearchRequest_mode>  SearchSearchRequest.propInfo['mode'].default;
        }
        if (values) {
            this['query'].setValues(values['query']);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'query-string': new FormControl(this['query-string'], [Validators.maxLength(256), ]),
                'from': new FormControl(this['from'], [maxValueValidator(1023), ]),
                'max-results': new FormControl(this['max-results'], [maxValueValidator(8192), ]),
                'sort-by': new FormControl(this['sort-by'], [Validators.maxLength(256), ]),
                'mode': new FormControl(this['mode'], [enumValidator(SearchSearchRequest_mode), ]),
                'query': this['query'].$formGroup,
            });
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['query-string'].setValue(this['query-string']);
            this._formGroup.controls['from'].setValue(this['from']);
            this._formGroup.controls['max-results'].setValue(this['max-results']);
            this._formGroup.controls['sort-by'].setValue(this['sort-by']);
            this._formGroup.controls['mode'].setValue(this['mode']);
            this['query'].setFormGroupValues();
        }
    }
}

