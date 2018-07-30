import { ISearchKindAggregation, SearchKindAggregation } from "./search-kind-aggregation.model";
import { ISearchCategoryAggregation, SearchCategoryAggregation } from "./search-category-aggregation.model";
import { SearchEntryList } from "./search-entry-list.model";
import { ISearchCategoryPreview, SearchCategoryPreview } from "./search-category-preview.model";
import { ISearchKindPreview, SearchKindPreview } from "./search-kind-preview.model";
import { SearchTenantAggregation, ISearchTenantAggregation } from "./search-tenant-aggregation.model";
import { ISearchTenantPreview, SearchTenantPreview } from "./search-tenant-preview.model";

export interface ISearchCategoryAggregationUI extends ISearchCategoryAggregation {
  'categories'?: { [key: string]: ISearchKindAggregation; };
}

export class SearchCategoryAggregationUI extends SearchCategoryAggregation implements ISearchCategoryAggregationUI {
  'categories': { [key: string]: ISearchKindAggregation; };
}


export interface ISearchCategoryPreviewUI extends ISearchCategoryPreview {
  'categories'?: { [key: string]: ISearchKindPreviewUI; };
}

export class SearchCategoryPreviewUI extends SearchCategoryPreview implements ISearchCategoryPreviewUI {
  'categories': { [key: string]: SearchKindPreviewUI; };
}


export interface ISearchKindAggregationUI extends ISearchKindAggregation {
  'kinds'?: { [key: string]: SearchEntryList };
}

export class SearchKindAggregationUI extends SearchKindAggregation implements ISearchKindAggregationUI {
  'kinds': { [key: string]: SearchEntryList };
}


export interface ISearchKindPreviewUI extends ISearchKindPreview {
  'kinds'?: { [key: string]: SearchEntryList };
}

export class SearchKindPreviewUI extends SearchKindPreview implements ISearchKindPreviewUI {
  'kinds': { [key: string]: SearchEntryList };
}


export interface ISearchTenantAggregationUI extends ISearchTenantAggregation {
  'tenants'?:  { [key: string]: ISearchCategoryAggregationUI; };
}

export class SearchTenantAggregationUI extends SearchTenantAggregation implements ISearchTenantAggregationUI {
  'tenants':  { [key: string]: SearchCategoryAggregationUI; };
}


export interface ISearchTenantPreviewUI extends ISearchTenantPreview {
  'tenants'?:  { [key: string]: ISearchCategoryPreviewUI; };
}

export class SearchTenantPreviewUI extends SearchTenantPreview implements ISearchTenantPreviewUI {
  'tenants':  { [key: string]: SearchCategoryPreviewUI; };
}