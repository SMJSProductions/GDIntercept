type Path = `#/${string}`;

interface BaseSchema<T extends string | number | boolean | never = never> {
    description?: string;
    default?: T;
    custom_default?: Path;
}

interface RefSchema extends BaseSchema<string | number | boolean | never> {
    $ref: Path;
}

interface OneOfSchema extends BaseSchema {
    oneOf: AnySchema[];
}

interface StringSchema extends BaseSchema<string> {
    type: "string";
    enum?: string[];
    minLength?: number;
    maxLength?: number;
    pattern?: string;
}

interface IntegerSchema extends BaseSchema<number> {
    type: "integer";
    minimum?: number;
    maximum?: number;
}

interface NumberSchema extends BaseSchema<number> {
    type: "number";
    minimum?: number;
    maximum?: number;
}

interface BooleanSchema extends BaseSchema<boolean> {
    type: "boolean";
}

interface ArraySchema extends BaseSchema {
    type: "array";
    minItems?: number;
    maxItems?: number;
    additionalItems?: boolean;
    items: AnySchema | AnySchema[];
}

interface ObjectSchema extends BaseSchema {
    type: "object";
    additionalProperties?: boolean;
    properties: {
        [key: string]: AnySchema;
    };
    required?: string[];
}

type AnySchema = StringSchema | NumberSchema | IntegerSchema | BooleanSchema | ArraySchema | ObjectSchema | OneOfSchema | RefSchema;

interface Schema extends ObjectSchema {
    $schema: string;
    $id: string;
    title: string;
    definitions?: {
        [key: string]: AnySchema;
    };
}