
import { SearchSearchRequest, SearchSearchResponse } from '@sdk/v1/models/generated/search';

export interface SearchSuggestion {
    name: string;
    label?: string;
    sample?: string;
    searchType: SearchsuggestionTypes;
    count?: number;
  }

export enum  SearchsuggestionTypes {
    KINDS= 'kinds',
    CATEGORIES= 'categories',
    INIT = 'init',
    OP_IN = 'in',
    OP_IS = 'is',
    OP_HAS = 'has',
    OP_TAG = 'tag',
    OP_TXT = 'txt'
}

export interface SearchSpec {
    in?: string;
    is?: string;
    has?: string;
    tag?: string;
    txt?: string[];
}

export interface SearchGrammarItem {
    type?: string;
    value?: string | string[];
}

export interface CompileSearchInputStringResult {
    input?: string;
    list?: any[];
    searchspec?: SearchSpec;
    freeformtext?: string[];
    error?: SearchInputErrors;
}

export interface SearchInputErrors extends SearchSpec {
    messages?: string[];
}

export interface SearchInputTypeValue {
    type: string;
    value: string;
}

export interface SearchExpression {
    key: string;
    operator: string;
    values: string[];
}

export interface SearchModelField {
    meta?: string[];
    spec?: string[];
    status?: string[];
}

// This interface defines search criteria confied in guided-search panel
export interface GuidedSearchCriteria {
    in?: string[];
    is?: string[];
    has?: any[];
    tag?: any[];
    txt?: any[];
}

// This interface define the objet search-result page will receive
export interface SearchResultPayload {
        id: string;
        result: SearchSearchResponse;
        searchstring: string;
        searchrequest: SearchSearchRequest;
}

export interface ExamineCategoryOrKindResult {
    output: any[];
    error: SearchInputErrors;
    type: string;
}
