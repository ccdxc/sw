/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { SearchSearchRequest_sort_order,  } from './enums';
import { SearchSearchRequest_mode,  } from './enums';
import { SearchSearchQuery, ISearchSearchQuery } from './search-search-query.model';

export interface ISearchSearchRequest {
    'query-string'?: string;
    'from': number;
    'max-results': number;
    'sort-by'?: string;
    'sort-order': SearchSearchRequest_sort_order;
    'mode': SearchSearchRequest_mode;
    'query'?: ISearchSearchQuery;
    'tenants'?: Array<string>;
    'aggregate'?: boolean;
}


export class SearchSearchRequest extends BaseModel implements ISearchSearchRequest {
    /** Simple query string
This can be specified as URI parameter.
For advanced query cases, Users should use specify SearchQuery
and pass the SearchRequest in a GET/POST Body
The max query-string length is 256 byteslength of string should be between 0 and 256 */
    'query-string': string = null;
    /** From represents the start offset (zero based), used in paginated search requests
The results returned would be in the range [From ... From+MaxResults-1]
This can be specified as URI parameter. 
Default value is 0 and valid range is 0..1023value should be between 0 and 1023 */
    'from': number = null;
    /** MaxResults is the max-count of search results
This can be specified as URI parameter.
Default value is 50 and valid range is 0..8192value should be between 0 and 8192 */
    'max-results': number = null;
    /** SortyBy is an optional parameter and contains the field name 
to be sorted by, For eg: "meta.name"
This can be specified as URI parameter. */
    'sort-by': string = null;
    /** SortOrder is an optional parameter and contains whether to sort ascending
or descending
This can be specified as URI parameter. */
    'sort-order': SearchSearchRequest_sort_order = null;
    /** Query Mode */
    'mode': SearchSearchRequest_mode = null;
    /** Search query contains the search requirements
This is intended for advanced query use cases involving
boolean query, structured term query and supports various
combinations of text, phrase strings and search modifiers
for specific categories, kinds, fields and labels.
This cannot be specified as URI parameter. */
    'query': SearchSearchQuery = null;
    /** OR of tenants within the scope of which search needs to be performed. If not specified, it will be set to tenant
of the logged in user */
    'tenants': Array<string> = null;
    /** Indicates whether to perform aggregation on the search results or not */
    'aggregate': boolean = null;
    public static propInfo: { [prop in keyof ISearchSearchRequest]: PropInfoItem } = {
        'query-string': {
            description:  'Simple query string This can be specified as URI parameter. For advanced query cases, Users should use specify SearchQuery and pass the SearchRequest in a GET/POST Body The max query-string length is 256 byteslength of string should be between 0 and 256',
            required: false,
            type: 'string'
        },
        'from': {
            description:  'From represents the start offset (zero based), used in paginated search requests The results returned would be in the range [From ... From+MaxResults-1] This can be specified as URI parameter.  Default value is 0 and valid range is 0..1023value should be between 0 and 1023',
            required: true,
            type: 'number'
        },
        'max-results': {
            default: parseInt('50'),
            description:  'MaxResults is the max-count of search results This can be specified as URI parameter. Default value is 50 and valid range is 0..8192value should be between 0 and 8192',
            required: true,
            type: 'number'
        },
        'sort-by': {
            description:  'SortyBy is an optional parameter and contains the field name  to be sorted by, For eg: &quot;meta.name&quot; This can be specified as URI parameter.',
            required: false,
            type: 'string'
        },
        'sort-order': {
            enum: SearchSearchRequest_sort_order,
            default: 'ascending',
            description:  'SortOrder is an optional parameter and contains whether to sort ascending or descending This can be specified as URI parameter.',
            required: true,
            type: 'string'
        },
        'mode': {
            enum: SearchSearchRequest_mode,
            default: 'full',
            description:  'Query Mode',
            required: true,
            type: 'string'
        },
        'query': {
            description:  'Search query contains the search requirements This is intended for advanced query use cases involving boolean query, structured term query and supports various combinations of text, phrase strings and search modifiers for specific categories, kinds, fields and labels. This cannot be specified as URI parameter.',
            required: false,
            type: 'object'
        },
        'tenants': {
            description:  'OR of tenants within the scope of which search needs to be performed. If not specified, it will be set to tenant of the logged in user',
            required: false,
            type: 'Array<string>'
        },
        'aggregate': {
            default: 'true',
            description:  'Indicates whether to perform aggregation on the search results or not',
            required: false,
            type: 'boolean'
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
                        SearchSearchRequest.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['query'] = new SearchSearchQuery();
        this['tenants'] = new Array<string>();
        this._inputValue = values;
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
        if (values && values['sort-order'] != null) {
            this['sort-order'] = values['sort-order'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('sort-order')) {
            this['sort-order'] = <SearchSearchRequest_sort_order>  SearchSearchRequest.propInfo['sort-order'].default;
        } else {
            this['sort-order'] = null
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
        if (values && values['aggregate'] != null) {
            this['aggregate'] = values['aggregate'];
        } else if (fillDefaults && SearchSearchRequest.hasDefaultValue('aggregate')) {
            this['aggregate'] = SearchSearchRequest.propInfo['aggregate'].default;
        } else {
            this['aggregate'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'query-string': CustomFormControl(new FormControl(this['query-string'], [maxLengthValidator(256), ]), SearchSearchRequest.propInfo['query-string']),
                'from': CustomFormControl(new FormControl(this['from'], [required, maxValueValidator(1023), ]), SearchSearchRequest.propInfo['from']),
                'max-results': CustomFormControl(new FormControl(this['max-results'], [required, maxValueValidator(8192), ]), SearchSearchRequest.propInfo['max-results']),
                'sort-by': CustomFormControl(new FormControl(this['sort-by'], [maxLengthValidator(256), ]), SearchSearchRequest.propInfo['sort-by']),
                'sort-order': CustomFormControl(new FormControl(this['sort-order'], [required, enumValidator(SearchSearchRequest_sort_order), ]), SearchSearchRequest.propInfo['sort-order']),
                'mode': CustomFormControl(new FormControl(this['mode'], [required, enumValidator(SearchSearchRequest_mode), ]), SearchSearchRequest.propInfo['mode']),
                'query': CustomFormGroup(this['query'].$formGroup, SearchSearchRequest.propInfo['query'].required),
                'tenants': CustomFormControl(new FormControl(this['tenants']), SearchSearchRequest.propInfo['tenants']),
                'aggregate': CustomFormControl(new FormControl(this['aggregate']), SearchSearchRequest.propInfo['aggregate']),
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('query') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('query').get(field);
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
            this._formGroup.controls['query-string'].setValue(this['query-string']);
            this._formGroup.controls['from'].setValue(this['from']);
            this._formGroup.controls['max-results'].setValue(this['max-results']);
            this._formGroup.controls['sort-by'].setValue(this['sort-by']);
            this._formGroup.controls['sort-order'].setValue(this['sort-order']);
            this._formGroup.controls['mode'].setValue(this['mode']);
            this['query'].setFormGroupValuesToBeModelValues();
            this._formGroup.controls['tenants'].setValue(this['tenants']);
            this._formGroup.controls['aggregate'].setValue(this['aggregate']);
        }
    }
}

