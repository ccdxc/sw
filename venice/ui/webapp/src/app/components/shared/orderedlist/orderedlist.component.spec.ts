import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { OrderedlistComponent } from './orderedlist.component';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { DragDropModule } from '@angular/cdk/drag-drop';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

describe('OrderedlistComponent', () => {
  let component: OrderedlistComponent;
  let fixture: ComponentFixture<OrderedlistComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ OrderedlistComponent ],
      imports: [
        NoopAnimationsModule,
        DragDropModule,
        MaterialdesignModule
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(OrderedlistComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
