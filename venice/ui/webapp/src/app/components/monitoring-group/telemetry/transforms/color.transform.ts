import { MetricTransform, TransformDataset, TransformDatasets } from './types';
import { Utility } from '@app/common/Utility';
import { sourceFieldKey } from '../utility';

export class ColorTransform extends MetricTransform {
  public static allColors = ['#97b8df', '#61b3a0', '#b592e3'];
  public static globalColorUseMap = {};
  // Maps from measurement-field to color used for that dataset
  colors: { [key: string]: string} = {};
  transformName = 'ColorTransform';

  public static getColor() {
    let color = this.allColors.find( (c) => {
      if (this.globalColorUseMap[c] == null || this.globalColorUseMap[c] === 0) {
        return true;
      }
      return false;
    });
    if (color == null) {
      // pick a random color
      const randInt = Utility.getRandomInt(0, this.allColors.length - 1); // both bounds are inclusive
      color = this.allColors[randInt];
    }
    if (this.globalColorUseMap[color] == null) {
      this.globalColorUseMap[color] = 0;
    }
    this.globalColorUseMap[color] += 1;
    return color;
  }

  public static releaseColor(color: string) {
    const colorCount = this.globalColorUseMap[color];
    if (colorCount == null || colorCount === 0) {
      return;
    }
    this.globalColorUseMap[color] -= 1;
  }

  constructor() {
    super();
  }

  getKey(opts: TransformDataset): string {
    // Since this key will only be used within this
    // color transform instance, source id key is not needed
    return sourceFieldKey('', opts.measurement, opts.field);
  }

  onFieldChange() {
    // If we assigned a field a color, it should be freed when a user deselects it.
    if (this.fields == null) {
      return;
    }
    const newColorsMap = {};
    this.fields.forEach( (f) => {
      const key = sourceFieldKey('', this.measurement, f);
      const color = this.colors[key];
      if (color != null) {
        newColorsMap[key] = color;
      }
    });
    // Free up any colors not being used anymore
    for (const [key, color] of Object.entries(this.colors)) {
      if (newColorsMap[key] == null) {
        // Color not being used
        ColorTransform.releaseColor(color);
      }
    }
    this.colors = newColorsMap;
  }

  transformDatasets(opts: TransformDatasets) {
    const newColorsMap = {};
    opts.forEach( (opt) => {
      // check for entry
      const key = this.getKey(opt);
      let currentColor = this.colors[key];
      if (currentColor == null) {
        // Need to get new color
        currentColor = ColorTransform.getColor();
      }
      newColorsMap[key] = currentColor;
      opt.dataset.backgroundColor = currentColor;
      opt.dataset.borderColor = currentColor;
    });

    // Free up any colors not being used anymore
    for (const [key, color] of Object.entries(this.colors)) {
      if (newColorsMap[key] == null) {
        // Color not being used
        ColorTransform.releaseColor(color);
      }
    }
    this.colors = newColorsMap;

  }

}
