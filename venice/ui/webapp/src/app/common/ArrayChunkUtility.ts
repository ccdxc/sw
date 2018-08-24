import { Utility } from '@app/common/Utility';

/**
 * This utility class allows for easily chunking data and
 * handling incoming data for PrimeNG Turbo table.
 *
 * Any data passed to the table will instantly be rendered. For efficiency,
 * we use lazy loading and simulate the chunking of the data. This utility
 * holds all the data, and requestChunk should be used to give primeNG only the chunks
 * it requests during the lazyLoad Event.
 *
 * This utility also handles holding data updates. When data comes in, the user
 * may not be ready to see it. Once the user is ready, we can use switchToNewData
 * to change the reference that is being used when data is requested.
 */
export class ArrayChunkUtility {
  currentDataArray: any[] = [];
  newDataArray: any[];
  lastRequestedIndex = {
    start: null,
    stop: null,
  };
  // Whether to instantly update the currentDataArray on updateData
  instantUpdate: boolean;

  public get length(): number {
    return this.currentDataArray.length;
  }

  /**
   *
   * @param instantUpdate Whether to automatically update
   *                      currentDataArray instead of holding the data
   *
   * @param data          initial data
   */
  constructor(instantUpdate: boolean, data: any[]) {
    this.instantUpdate = instantUpdate;
    this.currentDataArray = Utility.getLodash().cloneDeep(data);
  }

  /**
   * Returns a chunk of the current data
   * @param start
   * @param stop
   */
  requestChunk(start: number, stop: number): ReadonlyArray<any> {
    if (start == null || stop == null) {
      console.error('invalid chunk request');
      return [];
    }
    this.lastRequestedIndex = {
      start: start,
      stop: stop,
    };
    return this.currentDataArray.slice(start, stop);
  }

  /**
   * Returns the last request chunk on the current data source.
   *
   * This is useful for using after switching
   * data sources, and needing to replicate the last request
   * but with the new data.
   */
  getLastRequestedChunk(): ReadonlyArray<any> {
    const start = this.lastRequestedIndex.start;
    const stop = this.lastRequestedIndex.stop;
    return this.requestChunk(start, stop);
  }

  /**
   * If there are updates available, it will switch to it
   */
  switchToNewData() {
    if (this.newDataArray != null) {
      this.currentDataArray = this.newDataArray;
      this.newDataArray = null;
    }
  }

  updateData(data, switchNow = false) {
    this.newDataArray = Utility.getLodash().cloneDeep(data);
    if (this.instantUpdate || switchNow) {
      this.switchToNewData();
    }
  }

  enableInstantUpdate() {
    this.instantUpdate = true;
    this.switchToNewData();
  }

  disableInstantUpdate() {
    this.instantUpdate = false;
  }

  hasUpdate() {
    return this.newDataArray != null;
  }
}
