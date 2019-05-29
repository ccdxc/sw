import {SearchExpression} from '@components/search';

export interface AdvancedSearchExpression {
  searchExpressions: SearchExpression[];
  generalSearch: string[];
  remainingString?: string;
}

