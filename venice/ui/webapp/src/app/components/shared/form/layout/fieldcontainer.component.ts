import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-fieldcontainer',
  templateUrl: './fieldcontainer.component.html',
  styleUrls: ['./fieldcontainer.component.scss']
})
export class FieldContainerComponent {
  @Input() fieldTitle: string;
  @Input() subtitle: string;
  @Input() requiredLabel: boolean = false;
}
