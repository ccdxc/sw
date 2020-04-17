import { TableCol, CustomExportMap } from '.';
import { Utility } from '@app/common/Utility';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';
import { FieldsRequirement, IFieldsRequirement, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import * as _ from 'lodash';

/**
 * This is utility class handling commont table features, including export, search table, etc.
 */

export class TableUtility {

  /**
   * This object contains common function to filter data.
   * Table in UI page may contain advance search widget. AdvancedSearchComponent search criteria repeater operator specifies filter function.
   * This is the implementation of operator spec
   *
   */
  public static filterConstraints = {

    startsWith(value, filter): boolean {
      if (filter === undefined || filter === null || filter.trim() === '') {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      const filterValue = filter.toLowerCase();
      return value.toString().toLowerCase().slice(0, filterValue.length) === filterValue;
    },

    contains(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      return value.toString().toLowerCase().indexOf(filter.toLowerCase()) !== -1;
    },

    notcontains(value, filter): boolean {
      return !TableUtility.filterConstraints['contains'](value, filter);
    },

    endsWith(value, filter): boolean {
      if (filter === undefined || filter === null || filter.trim() === '') {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      const filterValue = filter.toString().toLowerCase();
      return value.toString().toLowerCase().indexOf(filterValue, value.toString().length - filterValue.length) !== -1;
    },

    equals(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      return value.toString().toLowerCase() === filter.toString().toLowerCase();
    },

    notEquals(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return false;
      }

      if (value === undefined || value === null) {
        return true;
      }

      return value.toString().toLowerCase() !== filter.toString().toLowerCase();
    },

    in(value, filter: any[]): boolean {
      if (filter === undefined || filter === null || filter.length === 0) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      for (let i = 0; i < filter.length; i++) {
        if (filter[i] === value) {
          return true;
        }
      }

      return false;
    },

    notin(value, filter: any[]): boolean {
      return !TableUtility.filterConstraints['in'](value, filter);
    },

    gt(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      if (Utility.isDateString(filter)) {
        value = value.split('.')[0];
        filter = filter.split('.')[0];
        return (new Date(value.toString())).getTime() > (new Date(filter.toString())).getTime();
      }
      return value > filter;
    },

    lt(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }

      if (Utility.isDateString(filter)) {
        value = value.split('.')[0];
        filter = filter.split('.')[0];
        return (new Date(value.toString())).getTime() < (new Date(filter.toString())).getTime();
      }
      return value < filter;
    },

    gte(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }
      value = value.split('.')[0];
      filter = filter.split('.')[0];
      return ((new Date(value.toString())).getTime() > (new Date(filter.toString())).getTime()) || (value.toString().toLowerCase()).includes(filter.toString().toLowerCase());
    },

    lte(value, filter): boolean {
      if (filter === undefined || filter === null || (typeof filter === 'string' && filter.trim() === '')) {
        return true;
      }

      if (value === undefined || value === null) {
        return false;
      }
      value = value.split('.')[0];
      filter = filter.split('.')[0];

      return ((new Date(value.toString())).getTime() < (new Date(filter.toString())).getTime()) || (value.toString().toLowerCase()).includes(filter.toString().toLowerCase());
    },

  };


  public static displayColumn(data, col, hasUiHintMap: boolean = true): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields, hasUiHintMap);
    const column = col.field;
    switch (column) {
      case 'modification-time':
      case 'creation-time':
        return new PrettyDatePipe('en-US').transform(value);
      default:
        return Array.isArray(value) ? JSON.stringify(value) : value;
    }
  }

  public static exportTableCSV(columns: TableCol[], data, filename: string, customExportMap: CustomExportMap = {}, csvSeparator = ',', hasUiHintMap: boolean = true) {
    const csv = TableUtility.buildCSV(columns, csvSeparator, data, customExportMap, hasUiHintMap);
    TableUtility.invokeDownload(csv.toString(), filename, 'csv');
  }
  public static exportTableJSON(columns: TableCol[], data, filename: string, customExportMap: CustomExportMap = {}, csvSeparator = ',', hasUiHintMap: boolean = true) {
    const json = TableUtility.buildJSON(columns, data, customExportMap, hasUiHintMap);
    TableUtility.invokeDownload(JSON.stringify(json, null, 5), filename, 'json');
  }

  public static buildCSV(columns: TableCol[], csvSeparator: string, data: any, customExportMap: CustomExportMap, hasUiHintMap: boolean) {
    let csv = '\ufeff';
    // headers
    for (let i = 0; i < columns.length; i++) {
      const column = columns[i];
      if (column.exportable !== false && column.field) {
        csv += '"' + (column.header || column.field) + '"';
        if (i < (columns.length - 1)) {
          csv += csvSeparator;
        }
      }
    }

    // body
    data.forEach((record, ii) => {
      csv += '\n';
      for (let i = 0; i < columns.length; i++) {
        const column = columns[i];
        if (column.exportable !== false && column.field) {
          let cellData = Utility.getLodash().get(record, column.field);
          // Priority 1) check for custom export settings
          const customExportFunc = customExportMap[column.field];
          if (customExportFunc) {
            cellData = customExportFunc({
              data: record,
              field: column.field
            });
          } else {
            //  Priority 2) keep value from table data
            if (!!cellData) {
              // Priority 3) fetch value from ui hint
              cellData = TableUtility.displayColumn(record, column, hasUiHintMap);
            }
          }
          if (cellData == null) {
            // Priority 4) if nothing else, set value to empty string
            cellData = '';
          }
          if (typeof cellData === 'object') {
            cellData = JSON.stringify(cellData).replace(/"/g, '""');
          } else {
            cellData = String(cellData).replace(/"/g, '""');
          }
          csv += '"' + cellData + '"';
          if (i < (columns.length - 1)) {
            csv += csvSeparator;
          }
        }
      }
    });
    return csv;
  }
  public static buildJSON(columns: TableCol[], data: any[], customExportMap: CustomExportMap, hasUiHintMap: boolean) {
    let csv = '\ufeff';
    const output = [];
    const headers: string[] = [];
    // headers
    for (let i = 0; i < columns.length; i++) {
      const column = columns[i];
      if (column.exportable !== false && column.field) {
        csv += '"' + (column.header || column.field) + '"';
        headers.push((column.header || column.field));
      }
    }
    // body
    data.forEach((record, ii) => {
      const rec = {};
      csv += '\n';
      for (let i = 0; i < columns.length; i++) {
        const column = columns[i];
        if (column.exportable !== false && column.field) {
          csv = TableUtility.buildJSONHelper(record, column, customExportMap, hasUiHintMap, csv, headers, i, rec);
        }
      }
      output.push(rec);
    });
    return output;
  }

  public static buildJSONHelper(record: any, column: TableCol, customExportMap: CustomExportMap, hasUiHintMap: boolean, csv: string, headers: string[], i: number, rec: {}) {
    let cellData = Utility.getLodash().get(record, column.field);
    // Priority 1) check for custom export settings
    const customExportFunc = customExportMap[column.field];
    if (customExportFunc) {
      cellData = customExportFunc({
        data: record,
        field: column.field
      });
    } else {
      //  Priority 2) keep value from table data
      if (!!cellData) {
        // Priority 3) fetch value from ui hint
        cellData = TableUtility.displayColumn(record, column, hasUiHintMap);
      }
    }
    if (cellData == null) {
      // Priority 4) if nothing else, set value to empty string
      cellData = '';
    }
    if (typeof cellData === 'object') {
      cellData = JSON.stringify(cellData).replace(/"/g, '""');
    } else {
      cellData = String(cellData).replace(/"/g, '""');
    }
    csv += '"' + cellData + '"';
    if (!Utility.isEmpty(headers[i])) {
      rec[headers[i]] = cellData;
    }
    return csv;
  }

  public static invokeDownload(content: string, filename: any, filetype: string = 'csv') {
    const blob = new Blob([content], {
      type: 'text/' + filetype + ';charset=utf-8;' // it looks like 'text/csv;charset=utf-8;'
    });
    if (window.navigator.msSaveOrOpenBlob) {
      navigator.msSaveOrOpenBlob(blob, filename + '.csv');
    } else {
      const link = document.createElement('a');
      link.style.display = 'none';
      document.body.appendChild(link);
      if (link.download !== undefined) {
        link.setAttribute('href', URL.createObjectURL(blob));
        link.setAttribute('download', filename + '.' + filetype); // xxx.csv
        link.click();
      } else {
        content = 'data:text/' + filetype + ';charset=utf-8,' + content; // looks like 'data:text/csv;charset=utf-8,' + csv;
        window.open(encodeURI(content));
      }
      document.body.removeChild(link);
    }
    return content;
  }

  /**
   * This API helps this.searchTableFields()
   * @param results
   * @param compareFunc
   */
  public static intersectionList(results: any[], compareFunc: (a, b) => boolean = _.isEqual): any[] {
    const len = results.length;
    let output: any[] = [];
    if (results.length === 1) {
      return results[0];
    }
    for (let i = len - 1; i >= 0; i--) {
      output = TableUtility.intersectionTwo(output, results[i], compareFunc);
    }
    return output;
  }

  /**
   * This API compute the intersection between tow data sets.
   * Internally, we use lodash.intersectionWith() api
   * Given tow data sets,  if one is empty, return the non-empty data set.  If both data sets are not empty, we use lodash.intersectionWith() api to compute the intersection
   * @param aList
   * @param bList
   * @param compareFunc
   */
  public static intersectionTwo(aList: any[], bList: any[], compareFunc: (a, b) => boolean): any[] {
    let output: any[];
    if ((!aList || aList.length === 0) && bList && bList.length > 0) {
      output = bList;
    } else if (aList && aList.length > 0 && (!bList || bList.length === 0)) {
      output = aList;
    } else if (aList && aList.length > 0 && bList && bList.length > 0) {
      output = _.intersectionWith(aList, bList, compareFunc);
    }
    return output;
  }

  /**
   * This is the core API of table search.  See src/app/components/workload/workload.component.ts WorkloadComponent.onSearchWorkloads()
   * UI page may contain an advance-search component.
   * For example: WorkloadComponent let user specify
   *  a. search with table meta data  (e.g search workload.meta.name = xxx)
   *  b. customize search  (e.g workload table contains DSC record which is not in workload object.  We must use customize search)
   *  c. text search  (free form text search)
   *
   * As WorkloadComponent constains all workload objects, we can search record locally.
   * This API has searchResults for a, customizeSearchResults  for b, textsearchResults for c
   *
   * The output should be the intersection among [a, b, c]
   * @param requirements
   * @param localRequirements
   * @param searchTexts
   * @param cols
   * @param data
   */
  public static searchTable(requirements: FieldsRequirement[], localRequirements: FieldsRequirement[], searchTexts: SearchTextRequirement[], cols: TableCol[], data: any[] | ReadonlyArray<any>) {
    if (!requirements.length && !localRequirements.length && !searchTexts.some(searchText => searchText.text.some((str: string) => str.trim().length > 0))) {
      return data;
    }

    let output: any[] | ReadonlyArray<any> = [];
    let searchResults: any[] | ReadonlyArray<any> = [];
    let customizeSearchResults: any[] | ReadonlyArray<any> = [];
    let textsearchResults: any[] | ReadonlyArray<any> = data;

    for (let i = 0; i < requirements.length; i++) {
      const req = requirements[i];
      // check for meta.creation time and meta.modification time
      if (Utility.isDateString(req.values[0])) {
        const timeFrame = (req.values[0]).split('.');
        req.values[0] = timeFrame[0];
      }
      // since requirements[i] are "AND" relations.  Find workload where  (meta.name = 'wl2' && spec.host='naples1')
      searchResults = (i === 0) ? TableUtility.searchTableOneField(req, data) : TableUtility.searchTableOneField(req, searchResults);
    }

    for (let j = 0; j < localRequirements.length; j++) {
      const reqCustomized: FieldsRequirement = localRequirements[j];
      const targetCol: TableCol = cols.find((col) => col.field === reqCustomized.key);
      if (targetCol.filterfunction) {
        const result = targetCol.filterfunction(reqCustomized, data);  // see WorkloadComponent.buildAdvSearchCols()  --> "filterfunction: this.searchDSC"
        customizeSearchResults = customizeSearchResults.concat(result);
      }
    }

    for (let k = 0; k < searchTexts.length; k++) {
      const searchText = searchTexts[k];
      textsearchResults = TableUtility.textSearchTableOne(searchText, textsearchResults);
    }
    output = TableUtility.intersectionList([searchResults, textsearchResults, customizeSearchResults]);
    return output;
  }

  /**
   * This function implement string match search.
   * @param searchText: SearchTextRequirement
   * @param data: any // e.g Workloads[]
   * @param recordToStringFunction is a function object. Given a record, make it a string. (text: any) => string. Default implementation is JSON.stringify(record),
   * @param trimTextValueFunction is a function object. Given a text, make it a string.
   */
  static textSearchTableOne(searchText: SearchTextRequirement, data: any[] | ReadonlyArray<any>,
    recordToStringFunction: (record: any) => string = (record) => JSON.stringify(record),
    trimTextValueFunction: (text: string) => string = (text) => text.replace(/"/g, '')
  ): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const recordValue = recordToStringFunction(data[i]);  // JSON.stringify(data[i]);
      const texts = searchText.text;
      for (let j = 0; texts && j < texts.length; j++)   {
        const text = trimTextValueFunction(texts[j]);
        const activateFunc = TableUtility.filterConstraints['contains'];
        if (activateFunc && activateFunc(recordValue, text) && !outputs.find(output => _.isEqual(output, data[i]))) {
          outputs.push(data[i]);
        }
      }
    }
    return outputs;
  }

  /**
   * This API searches records.  Say we have workloads and we want search meta.name="wl2".  This api will loop all workload records to see if workloads[i].meta.name === 'wl2"
   * @param requirement : FieldsRequirement
   * @param data: any[]. e.g workloads[]
   * @param delimiter : string. default is "."
   */
  public static searchTableOneField(requirement: FieldsRequirement, data: any[] | ReadonlyArray<any>, delimiter: string = '.'): any[] {
    const outputs: any[] = [];
    const isSearchingLabels = (requirement.key === 'meta.labels');
    const fields: Array<string> = requirement.key.split(delimiter);
    const searchValues = requirement.values;
    let operator = String(requirement.operator);
    operator = TableUtility.convertOperator(operator);
    for (let i = 0; data && i < data.length; i++) {
      const recordValue = _.get(data[i], fields);
      for (let j = 0; searchValues && j < searchValues.length; j++) {
        const activateFunc = TableUtility.filterConstraints[operator];
        if (!isSearchingLabels) {
          if (Array.isArray(recordValue)) {
            for (let k = 0; k < recordValue.length; k++) {
              if (activateFunc && activateFunc(recordValue[k], searchValues[j])) {
                outputs.push(data[i]);
              }
            }
          } else {
            if (activateFunc && activateFunc(recordValue, searchValues[j])) {
              outputs.push(data[i]);
            }
          }
        } else {
          //  When seaching meta.labels, recordValue looks like "{"env":"21","tag":"1"}"
          const labelKeys = Object.keys(recordValue || {});
          for (let l = 0; labelKeys && l < labelKeys.length; l++) {
            const searchInputs = searchValues[j].split(':');  // searchInputs can be env:21, or just "21"
            if (searchInputs.length === 2) {
              const searchKey = searchInputs[0].trim();
              const searchVal = searchInputs[1].trim();
              if (activateFunc && activateFunc(labelKeys[l], searchKey) && activateFunc(recordValue[labelKeys[l]], searchVal)) {
                outputs.push(data[i]);
              }
            } else {
              const searchVal = searchInputs[0];
              if (activateFunc && activateFunc(recordValue[labelKeys[l]], searchVal)) {
                outputs.push(data[i]);
              }
            }
          }
        }
      }
    }
    return outputs;

  }

  public static convertOperator(operator: string): string {
    if (!operator) {
      return 'contains';
    }

    const operatorStr = operator.toLocaleLowerCase();
    switch (operatorStr) {
      case 'notin':
      case 'not equals':
        return 'notcontains';
      case 'in':
        return 'contains';
      case 'gt':
        return 'gt';
      case 'gte':
        return 'gte';
      case 'lt':
        return 'lt';
      case 'lte':
        return 'lte';
      default:
        return 'contains';
    }
  }
}
