import { MetricTransform, TransformDataset, TransformDatasets, TransformNames } from './types';
import { Utility } from '@app/common/Utility';
import { sourceFieldKey } from '../utility';

interface ColorTransformConfig {
  // This variable is shared by all color transforms
  globalColorUseMap: { [color: string]: number };
  // Maps from measurement-field to color used for that dataset
  colors: { [key: string]: string};
}

export class ColorTransform extends MetricTransform<ColorTransformConfig> {
  public static allColors = Utility.allColors;

  public static globalColorUseMap: { [color: string]: number } = {};
  // Maps from measurement-field to color used for that dataset
  colors: { [key: string]: string} = {};
  transformName = TransformNames.ColorTransform;

  public static getColor() {
    if (!this.globalColorUseMap) {
      this.globalColorUseMap = {};
    }

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
    const tags: { [key: string]: string} = opts.series.tags as any;
    let ret = sourceFieldKey('', opts.measurement, opts.field);
    if (tags != null) {
      let tagKey = '';
      Object.keys(tags).forEach( key => {
        tagKey += key + ':' + tags[key] + ' ';
      });
      if (tagKey.length > 0) {
        ret += '-' + tagKey;
      }
    }
    return ret;
  }

  onFieldChange() {
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

  load(config: ColorTransformConfig) {
    if (config  != null) {
      ColorTransform.globalColorUseMap = config.globalColorUseMap;
      this.colors = config.colors;
    }
  }

  save(): ColorTransformConfig {
    return {
      globalColorUseMap: ColorTransform.globalColorUseMap,
      colors: this.colors
    };
  }

}
