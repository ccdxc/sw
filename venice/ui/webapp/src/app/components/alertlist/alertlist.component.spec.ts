import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { AlertlistComponent } from './alertlist.component';
import { AlertlistitemComponent } from './alertlistitem.component';
import { SharedModule } from '@app/components/shared//shared.module';

describe('AlertlistComponent', () => {
  let component: AlertlistComponent;
  let fixture: ComponentFixture<AlertlistComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        AlertlistComponent,
        AlertlistitemComponent
      ],
      imports: [
        // Other modules...
        SharedModule
      ],
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertlistComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
