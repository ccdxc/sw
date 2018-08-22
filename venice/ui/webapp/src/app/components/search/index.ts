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
    OP_IN= 'in',
    OP_IS = 'is',
    OP_HAS = 'has',
    OP_TAG= 'tag'
}

export interface SearchSpec {
    in?: string;
    is?: string;
    has?: string;
    tag?: string;
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

// This interface define search criteria confied in guided-search panel
export interface GuidedSearchCriteria {
    in?: string[];
    is?: string[];
    has?: any[];
    tag?: any[];
}
