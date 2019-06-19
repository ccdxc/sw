import { FormControl } from '@angular/forms';
import { GraphTransform, TransformNames } from './types';

interface GraphTitleTransformConfig {
  title: string;
}

export class GraphTitleTransform extends GraphTransform<GraphTitleTransformConfig> {
  transformName = TransformNames.GraphTitle;

  title: FormControl = new FormControl('');

  constructor() {
    super();
  }

  load(config: GraphTitleTransformConfig) {
    if (config == null || config.title == null) {
      return;
    }
    this.title.setValue(config.title);
  }

  save(): GraphTitleTransformConfig {
    return {
      title: this.title.value
    };
  }

}

